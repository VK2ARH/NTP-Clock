//#include <Arduino_JSON.h>

/**************************************************************************
       Title:   NTP Dual Clock
      Author:   Bruce E. Hall, w8bh.net
        Date:   13 Feb 2021
    Hardware:   HiLetGo ESP32 or D1 Mini ESP8266, ILI9341 TFT display
    Software:   Arduino IDE 1.8.13 with Expressif ESP32 package 
                TFT_eSPI Library
                ezTime Library
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Dual UTC/Local NTP Clock with TFT display 
                Time is refreshed via NTP every 30 minutes
                Optional time output to serial port 
                Status indicator for time freshness & WiFi strength

                Before using, please update WIFI_SSID and WIFI_PWD
                with your personal WiFi credentials.  Also, modify
                TZ_RULE with your own Posix timezone string.

                see w8bh.net for a detailled, step-by-step tutorial

  Version Hx:   11/27/20  Initial GitHub commit
                11/28/20  added code to handle dropped WiFi connection
                11/30/20  showTimeDate() mod by John Price (WA2FZW)
                12/01/20  showAMPM() added by John Price (WA2FZW)
                02/05/21  added support for ESP8266 modules
                02/07/21  added day-above-month option
                02/10/21  added date-leading-zero option
                04/19/25  added WifiManager processes mod by Simon Mitchell (VK2YU)
               
 **************************************************************************/

#include <FS.h>                                    // to be used later -VK2YU
#include <TFT_eSPI.h>                              // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                                // https://github.com/ropg/ezTime
#if defined(ESP32)
#include <WiFi.h>                                  // use this WiFi lib for ESP32, or
#elif defined (ESP8266)
#include <ESP8266WiFi.h>                           // use this WiFi lib for ESP8266
#include <DNSServer.h>                             // requiresd for WifiManager
#include <ESP8266WebServer.h>                      // requiresd for WifiManager
#include <WiFiManager.h>                           // requiresd for WifiManager
#include <ArduinoJson.h>                           // to be used later -VK2YU
#include <ESP8266HTTPClient.h>                     // used for collection of solar data https://github.com/rkincaid/NTP_DualClock_Solar/tree/main
#endif

#define TITLE              "NTP Clock"    
#define WIFISTART          "Wifi Connecting..."
//#define WIFI_SSID        ""                       // disabled due to using WifiManager
//#define WIFI_PWD         ""                       // disabled due to using WifiManager
#define NTP_SERVER         "pool.ntp.org"           // time.nist.gov, pool.ntp.org, etc    
// #define saveConfigCallback    ""
#define TZ_RULE            "AEST-10AEDT,M10.1.0,M4.1.0/3"
//#define TZ_RULE            "CST6CDT,M3.2.0,M11.1.0" // Centeral Standard Time USA

#define SW_URL "https://www.hamqsl.com/solarxml.php"  // xml data from hamqsl
#define SCREEN_ORIENTATION       1                 // screen portrait mode:  use 1 or 3
#define LED_PIN                  2                 // built-in LED is on GPIO 2
#define DEBUGLEVEL            INFO                 // NONE, ERROR, INFO, or DEBUG
#define PRINTED_TIME             1                 // 0=NONE, 1=UTC, or 2=LOCAL
#define TIME_FORMAT         COOKIE                 // COOKIE, ISO8601, RFC822, RFC850, RFC3339, RSS
#define BAUDRATE            115200                 // serial output baudrate																			 
#define SYNC_MARGINAL         3600                 // orange status if no sync for 1 hour
#define SYNC_LOST            86400                 // red status if no sync for 1 day
#define LOCAL_FORMAT_12HR     true                 // local time format 12hr "11:34" vs 24hr "23:34"
#define UTC_FORMAT_12HR      false                 // UTC time format 12 hr "11:34" vs 24hr "23:34"
#define DISPLAY_AMPM         true                 // if true, show 'A' for AM, 'P' for PM  
#define HOUR_LEADING_ZERO    false                 // "01:00" vs " 1:00"
#define DATE_LEADING_ZERO     true                 // "Feb 07" vs. "Feb 7"
#define DATE_ABOVE_MONTH      true                 // "12 Feb" vs. "Feb 12" 
#define TIMECOLOR        TFT_GREEN                 // color of 7-segment time display
#define DATECOLOR         TFT_GOLD                 // color of displayed month & day
#define LABEL_FGCOLOR     TFT_GOLD                 // color of label text
#define LABEL_BGCOLOR     TFT_NAVY                 // color of label background


// ============ GLOBAL VARIABLES =====================================================

TFT_eSPI tft = TFT_eSPI();                         // display object 
Timezone local;                                    // local timezone variable
time_t t,oldT;                                     // current & displayed UTC
time_t lt,oldLt;                                   // current & displayed local time  
bool useLocalTime  = false;                        // temp flag used for display updates
WiFiServer server(80);                             // to be used later -VK2YU
String header;                                     // to be used later -VK2YU
char* solar_cond = (char*)calloc(32, sizeof(char));  // buffer to hold space weather info
int solar_min = 32;                                  // the minute each hour we'll update
int solar_sec = 0;                                   // the second each hour we'll update
bool shouldSaveConfig = false;                     // flag for saving data -to be used later -VK2YU
// you need the figerprint below to make an https connection to hamqsl
// if it fails check the certificate on hamqsl (URL above) to see if it changed
const char fingerprint[] PROGMEM = "9F:E6:07:3E:9E:59:09:67:06:6F:F0:E1:2B:34:B1:55:3F:7D:86:4E";

// ============ DISPLAY ROUTINES =====================================================

void showClockStatus() {
  const int x=290,y=1,w=28,h=29,f=2;               // screen position & size
  int color;
  if (second()%10) return;                         // update every 10 seconds
  int syncAge = now()-lastNtpUpdateTime();         // how long has it been since last sync?
  if (syncAge < SYNC_MARGINAL)                     // GREEN: time is good & in sync
    color = TFT_GREEN;
  else if (syncAge < SYNC_LOST)                    // ORANGE: sync is 1-24 hours old
    color = TFT_ORANGE;
  else color = TFT_RED;                            // RED: time is stale, over 24 hrs old
  if (WiFi.status()!=WL_CONNECTED) {               //          
    color = TFT_DARKGREY;                          // GRAY: WiFi connection was lost
    WiFi.disconnect();                             // so drop current connection
    // WiFi.begin(WIFI_SSID,WIFI_PWD);             // and attempt to reconnect
    delay(2000);                                   // allow time for the wifi to disconnect cleanly
    ESP.restart();                                 // forces the clock to restart and wifimanger will reconnect or revert AP mode
  }
  tft.fillRoundRect(x,y,w,h,10,color);             // show clock status as a color
  tft.setTextColor(TFT_BLACK,color);
  tft.drawNumber(-WiFi.RSSI(),x+8,y+6,f);          // WiFi strength as a positive value
}

/*
 *  Modified by John Price (WA2FZW)
 *
 *    In the original code, this was an empty function. I added code to display either
 *    an 'A' or 'P' to the right of the local time 
 *
 */

void showAMPM ( int hr, int x, int y )
{
  char  ampm;                                      // Will be either 'A' or 'P'
  if ( hr <= 11 )                                  // If the hour is 11 or less
    ampm = 'A';                                    // It's morning
  else                                             // Otherwise,
    ampm = 'P';                                    // It's afternoon
  tft.drawChar ( ampm, x, y, 4 );                  // Show AM/PM indicator 
}

void showTime(time_t t, bool hr12, int x, int y) {
  const int f=7;                                   // screen font
  tft.setTextColor(TIMECOLOR, TFT_BLACK);          // set time color
  int h=hour(t); int m=minute(t); int s=second(t); // get hours, minutes, and seconds
  if (hr12) {                                      // if using 12hr time format,
    if (DISPLAY_AMPM) showAMPM(h,x+220,y+14);      // show AM/PM indicator
    if (h==0) h=12;                                // 00:00 becomes 12:00
    if (h>12) h-=12;                               // 13:00 becomes 01:00
  }
  if (h<10) {                                      // is hour a single digit?
    if ((!hr12)||(HOUR_LEADING_ZERO))              // 24hr format: always use leading 0
      x+= tft.drawChar('0',x,y,f);                 // show leading zero for hours
    else {
      tft.setTextColor(TFT_BLACK,TFT_BLACK);       // black on black text     
      x+=tft.drawChar('8',x,y,f);                  // will erase the old digit
      tft.setTextColor(TIMECOLOR,TFT_BLACK);      
    }
  }
  x+= tft.drawNumber(h,x,y,f);                     // hours
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (m<10) x+= tft.drawChar('0',x,y,f);           // leading zero for minutes
  x+= tft.drawNumber(m,x,y,f);                     // show minutes          
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (s<10) x+= tft.drawChar('0',x,y,f);           // add leading zero for seconds
  x+= tft.drawNumber(s,x,y,f);                     // show seconds
}

void showDate(time_t t, int x, int y) {
  const int f=4,yspacing=30;                       // screen font, spacing
  const char* months[] = {"JAN","FEB","MAR",
     "APR","MAY","JUN","JUL","AUG","SEP","OCT",
     "NOV","DEC"};
  tft.setTextColor(DATECOLOR, TFT_BLACK);
  int i=0, m=month(t), d=day(t);                   // get date components 
  tft.fillRect(x,y,50,60,TFT_BLACK);               // erase previous date
  if (DATE_ABOVE_MONTH) {                          // show date on top -----
    if ((DATE_LEADING_ZERO) && (d<10))             // do we need a leading zero?
      i = tft.drawNumber(0,x,y,f);                 // draw leading zero
    tft.drawNumber(d,x+i,y,f);                     // draw date 
    y += yspacing;                                 // and below it, the month
    tft.drawString(months[m-1],x,y,f);             // draw month
  } else {                                         // put month on top ----
    tft.drawString(months[m-1],x,y,f);             // draw month
    y += yspacing;                                 // and below it, the date
    if ((DATE_LEADING_ZERO) && (d<10))             // do we need a leading zero?
      x+=tft.drawNumber(0,x,y,f);                  // draw leading zero
    tft.drawNumber(d,x,y,f);                       // draw date
  }
}

void showTimeZone (int x, int y) {
  const int f=4;                                   // text font
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set text colors
  tft.fillRect(x,y,80,28,LABEL_BGCOLOR);           // erase previous TZ
  if (!useLocalTime) 
    tft.drawString("UTC",x,y,f);                   // UTC time
  else 
    tft.drawString(local.getTimezoneName(),x,y,f); // show local time zone
}

void showTimeDate(time_t t, time_t oldT, bool hr12, int x, int y) {   
  showTime(t,hr12,x,y);                            // display time HH:MM:SS 
  if ((!oldT)||(hour(t)!=hour(oldT)))              // did hour change?
    showTimeZone(x,y-42);                          // update time zone
  if ((!oldT)||(day(t)!=day(oldT)))                // did date change? (Thanks John WA2FZW!)
    showDate(t,x+250,y);                           // update date
}

void updateDisplay() {
  t = now();                                       // check latest time
  if (t!=oldT) {                                   // are we in a new second yet?
    lt = local.now();                              // keep local time current
    useLocalTime = true;                           // use local timezone
    showTimeDate(lt,oldLt,LOCAL_FORMAT_12HR,10,46);// show new local time
    useLocalTime = false;                          // use UTC timezone
    showTimeDate(t,oldT,UTC_FORMAT_12HR,10,172);   // show new UTC time
    showClockStatus();                             // and clock status
    printTime();                                   // send timestamp to serial port
    oldT=t; oldLt=lt;                              // remember currently displayed time

    // get solar data first time or on the hour at solar_min
    if (((minute(t) == solar_min) && second(t) == solar_sec) || *solar_cond == 0) {
      tft.setTextColor(LABEL_BGCOLOR, LABEL_BGCOLOR);  // set label colors
      tft.drawString(solar_cond, 80, 130, 4);          // clear previous data
      getSolarData();
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);  // set label colors
      printTime();
      tft.drawString(solar_cond, 80, 130, 4);
      Serial.println(solar_cond);
    }



  }
}

void newDualScreen() {
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  tft.fillRoundRect(0,0,319,32,10,LABEL_BGCOLOR);  // title bar for local time
  tft.fillRoundRect(0,126,319,32,10,LABEL_BGCOLOR);// title bar for UTC
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set label colors
  tft.drawCentreString(TITLE,160,4,4);             // show title at top
  tft.drawRoundRect(0,0,319,110,10,TFT_GOLD);      // draw edge around local time
  tft.drawRoundRect(0,126,319,110,10,TFT_GOLD);    // draw edge around UTC
}

void startupScreen() {  
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  tft.fillRoundRect(0,0,319,30,10,LABEL_BGCOLOR);  // title bar
  tft.drawRoundRect(0,0,319,239,10,TFT_WHITE);     // draw edge screen
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set label colors
  tft.drawCentreString(WIFISTART,160,2,4);         // show sketch title on screen
  tft.setTextColor(LABEL_FGCOLOR, TFT_BLACK);      // set text color
}


// ============ MISC ROUTINES =====================================================

/*void saveConfigCallback () {                      // used for saving config items if set
  shouldSaveConfig = true;
}
*/
void showConnectionProgress(){  
  int elapsed = 0;           
  tft.drawString("WiFi connected at: ",5,50,4);                   
  while (WiFi.status()!=WL_CONNECTED) {            // while waiting for connection                             
    tft.drawNumber(elapsed++,230,50,4);            // show we are trying!
    delay(1000);
  }
  tft.drawString("IP = " +                         // connected to LAN now
    WiFi.localIP().toString(),5,80,4);             // so show IP address 
  elapsed = 0;
  tft.drawString("Waiting for NTP",5,140,4);       // Now get NTP info
  while (timeStatus()!=timeSet) {                  // wait until time retrieved
    events();                                      // allow ezTime to work
    tft.drawNumber(elapsed++,230,140,4);           // show we are trying
    delay(1000);
  }
}

void printTime() {                                 // print time to serial port
  if (!PRINTED_TIME) return;                       // option 0: dont print
  Serial.print("TIME: ");
  if (PRINTED_TIME==1)
    Serial.println(dateTime(TIME_FORMAT));         // option 1: print UTC time
  else 
    Serial.println(local.dateTime(TIME_FORMAT));   // option 2: print local time
}

void blink(int count=1) {                          // diagnostic LED blink
  pinMode(LED_PIN,OUTPUT);                         // make sure pin is an output
  for (int i=0; i<count; i++) {                    // blink counter
    digitalWrite(LED_PIN,0);                       // turn LED on 
    delay(200);                                    // for 0.2s
    digitalWrite(LED_PIN,1);                       // and then turn LED off
    delay(200);                                    // for 0.2s
  } 
  pinMode(LED_PIN,INPUT);                          // works for both Vcc & Gnd LEDs.
}

/*
 * poor man's xml tag extraction. I didn't want to
 * pull in an entire XML library when it wasn't really
 * that necessary. It's pretty straightforward. Just
 * use indexof to find the tag locations and pull out
 * the value with substring.
*/
String getXmlData(String xml, String tag) {
  int i = xml.indexOf("<" + tag + ">");
  int j = xml.indexOf("</" + tag + ">");
  if (i > 0 && j > i && j < xml.length()) {
    String val = xml.substring(i + tag.length() + 2, j);
    val.trim();
    return val;
  } else {
    return "??";  // if we didn't find anything then use this
  }
}

/*
 * Function added to grab solar conditions as XML from:
 * https://www.hamqsl.com/solarxml.php
 * only SFI, A and K are displayed, but other items
 * could be added later. 
 * The connection is made with HTTPS encryption
 * which requires a "fingerprint" included at the
 * beginning of this code. *IF* hamqsl updates their
 * HTTPS certificate you made need to update the 
 * fingerprint.
 *
 * solar mods by AI6P
 */
void getSolarData() {
  WiFiClientSecure client;
  HTTPClient http;
  client.setFingerprint(fingerprint);      // set the HTTPS fingerprint
  http.begin(client, SW_URL);              // open the URL connection
  int httpResponseCode = http.GET();       // get the response code
  sprintf(solar_cond, "*** no data ***");  // init the report in case if fails
  for (int i = 0; i < 5; ++i) {            // try up to 5 times
    if (httpResponseCode > 0) {            // if we got a response try to use it
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();  // get the XML
      //Serial.println(payload);
      String sflux = getXmlData(payload, "solarflux");  // find the solar flux
      String kindx = getXmlData(payload, "kindex");     // find the k index
      String aindx = getXmlData(payload, "aindex");     // find the aindex
      sprintf(solar_cond, "  SFI: %s  A: %s  K: %s", sflux, aindx, kindx);
      break;  // ok we're good, don't retry
    } else {
      Serial.print("Error code: ");      // print the response code to the console
      Serial.println(httpResponseCode);  // if something goes wrong
    }
    delay(100);
  }
  // Free resources
  http.end();
}

// ============ MAIN PROGRAM ===================================================

void setup() {
  tft.init();                                      // initialize LCD screen object
  tft.setRotation(SCREEN_ORIENTATION);             // landscape screen orientation
  startupScreen();                                 // show Wifi is starting
  blink(3);                                        // show sketch is starting
  Serial.begin(BAUDRATE);                          // open serial port
  setDebug(DEBUGLEVEL);                            // enable NTP debug info
  setServer(NTP_SERVER);                           // set NTP server
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  local.setPosix(TZ_RULE);                         // estab. local TZ by rule 
  //WifiManagerParameter customCallSign("callSign", "Enter your Callsign: ", callSign, 8);  //New object to collect -Callsign of upto 8 characters

  WiFiManager wifiManager;                         // starts the wifimanager. To capture WiFi details if not already
  //wifiManager.setSaveConfigCallback(saveConfigCallback);  // adds the process to capture and save additional variables
  //wifiManager.addParameter(&customCallSign);       // adds the custom callsign parameter to collect
  
  wifiManager.autoConnect("NTPClockAP");          // set the name of the AccessPoint for WifiManager
  server.begin();                                   // starts a simple webserver to capture user input for wifi 
 //strcpy(callSign, customCallSign.getValue());    //copy values to memory from Wifimanger
 // if (shouldSaveConfig) { 
 // saveConfig();
 // }
 
  //TITLE = callSign();                            // to be used later -VK2YU
  //WiFi.begin(WIFI_SSID, WIFI_PWD);               // start WiFi (redundant since wifimanager)
  showConnectionProgress();                        // WiFi and NTP may take time
  delay(2000);
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  newDualScreen();                                 // show title & labels 
  *solar_cond = 0;                      // initialize the conition buffer to be a null string
  solar_sec = second(local.now());      // we'll poll on this second as pseudo random
  solar_min = (solar_sec) % 5 + 32;     // we'll poll some time in the first 5 minutes 
                                        // or so after the half hour.
                                        // This is to spread out other clocks for load
                                        // balancing. 
}

void loop() {
  events();                                        // get periodic NTP updates
  updateDisplay();                                 // update clock every second
}

