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
                07/30/25  added brightness control (1 thru 7 + auto, selected by SW2) by Mark Culross (KD5RXT)
                07/31/25  fixed blank screen when brightness == auto by Mark Culross (KD5RXT)
                08/06/25  added "day mode" / "night mode" toggle with long press (>= 3 secs) of SW2 by Mark Culross (KD5RXT)
                14Aug2025 changed secure connection method to hamqsl to root cert rather than server cert due to cert expiry periods Simon Mitchell (VK2YU)  
               
 **************************************************************************/

#include <FS.h>        // to be used later -VK2YU
#include <TFT_eSPI.h>  // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>    // https://github.com/ropg/ezTime
#if defined(ESP32)
#include <WiFi.h>  // use this WiFi lib for ESP32, or
#elif defined(ESP8266)
#include <ESP8266WiFi.h>        // use this WiFi lib for ESP8266
#include <DNSServer.h>          // requiresd for WifiManager
#include <ESP8266WebServer.h>   // requiresd for WifiManager
#include <WiFiManager.h>        // requiresd for WifiManager
#include <ArduinoJson.h>        // to be used later -VK2YU
#include <WiFiClientSecure.h>   // used for secure TLS connections VK2YU
#include <ESP8266HTTPClient.h>  // used for collection of solar data https://github.com/rkincaid/NTP_DualClock_Solar/tree/main
#endif

// begin additions by KD5RXT 20250730
#include <Button.h>

#define SW2_PIN D6
Button SW2(SW2_PIN);  // SW2 button connected between SW2_PIN and GND

#define LED_BACKLIGHT_PIN D0
#define LIGHT_SENSOR_PIN A0
#define CHECK_LIGHT_SENSOR_INTERVAL_IN_MILLIS 50

// begin additions by KD5RXT 20250806
#define CHECK_BUTTON_LONG_PRESS_INTERVAL_IN_MILLIS 1000
#define LONG_PRESS_DURATION_IN_SECONDS 3

unsigned int press_duration = 0;
boolean night_mode = false;
// end additions by KD5RXT 20250806

unsigned long check_button_hold_interval = millis();
unsigned long check_light_sensor_interval = millis();

byte brightness = 224;

typedef enum {
   BUTTON_MODE_IDLE = 0,
   BUTTON_MODE_PRESSED,
   BUTTON_MODE_ACTIVATED,
} BUTTON_MODE_TYPE;

BUTTON_MODE_TYPE SW2_state = BUTTON_MODE_IDLE;
// end additions by KD5RXT 20250730

#define TITLE "NTP Clock"
#define INITIAL "Initializing..."
#define WIFISTART "Wifi Connecting..."
// #define WIFI_SSID          ""                       // disabled due to using WifiManager
// #define WIFI_PWD           ""                       // disabled due to using WifiManager
#define NTP_SERVER "pool.ntp.org"  // time.nist.gov, pool.ntp.org, etc    *
// #define saveConfigCallback    ""
#define TZ_RULE            "AEST-10AEDT,M10.1.0,M4.1.0/3"
//#define TZ_RULE "CST6CDT,M3.2.0,M11.1.0"  // Centeral Standard Time USA

#define SW_URL "https://www.hamqsl.com/solarxml.php"  // xml data from hamqsl
#define SCREEN_ORIENTATION 1                          // screen portrait mode:  use 1 or 3
#define LED_PIN 2                                     // built-in LED is on GPIO 2
#define DEBUGLEVEL INFO                               // NONE, ERROR, INFO, or DEBUG
#define PRINTED_TIME 1                                // 0=NONE, 1=UTC, or 2=LOCAL
#define TIME_FORMAT COOKIE                            // COOKIE, ISO8601, RFC822, RFC850, RFC3339, RSS
#define BAUDRATE 115200                               // serial output baudrate
#define SYNC_MARGINAL 3600                            // orange status if no sync for 1 hour
#define SYNC_LOST 86400                               // red status if no sync for 1 day
#define LOCAL_FORMAT_12HR false                       // local time format 12hr "11:34" vs 24hr "23:34"
#define UTC_FORMAT_12HR false                         // UTC time format 12 hr "11:34" vs 24hr "23:34"
#define DISPLAY_AMPM false                            // if true, show 'A' for AM, 'P' for PM
#define HOUR_LEADING_ZERO true                        // "01:00" vs " 1:00"
#define DATE_LEADING_ZERO true                        // "Feb 07" vs. "Feb 7"
#define DATE_ABOVE_MONTH true                         // "12 Feb" vs. "Feb 12"
#define TIMECOLOR TFT_GREEN                           // color of 7-segment time display
#define DATECOLOR TFT_GOLD                            // color of displayed month & day
#define LABEL_FGCOLOR TFT_GOLD                        // color of label text
#define LABEL_BGCOLOR TFT_NAVY                        // color of label background
// begin additions by KD5RXT 20250806
#define TIMECOLOR_NIGHT TFT_RED                       // color of 7-segment time display - night mode
#define DATECOLOR_NIGHT TFT_RED                       // color of displayed month & day - night mode
#define LABEL_FGCOLOR_NIGHT TFT_RED                  // color of label text - night mode
#define LABEL_BGCOLOR_NIGHT TFT_BLACK                 // color of label background - night mode
// end additions by KD5RXT 20250806


// ============ GLOBAL VARIABLES =====================================================

TFT_eSPI tft = TFT_eSPI();                           // display object
Timezone local;                                      // local timezone variable
time_t t, oldT;                                      // current & displayed UTC
time_t lt, oldLt;                                    // current & displayed local time
bool useLocalTime = false;                           // temp flag used for display updates
WiFiServer server(80);                               // to be used later -VK2YU
String header;                                       // to be used later -VK2YU
char* solar_cond = (char*)calloc(32, sizeof(char));  // buffer to hold space weather info
int solar_min = 32;                                  // the minute each hour we'll update
int solar_sec = 0;                                   // the second each hour we'll update
bool shouldSaveConfig = false;                       // flag for saving data -to be used later -VK2YU
// you need the fingerprint below to make an https connection to hamqsl
// if it fails check the certificate on hamqsl (URL above) to see if it changed
// begin additions by KD5RXT 20250730
// const char fingerprint[] PROGMEM = "4C:B3:62:99:05:CD:85:7F:53:A4:8B:C8:CA:C5:77:A7:D9:5E:40:08";  // valid thru 26 Oct 2025
// end additions by KD5RXT 20250730
//
// begin root cert checking updates by VK2YU 14Aug2025 
// (commented out old server SSL certs above) -Yes this behemoth const should probably be cut out to a seperate .h file but I'm lazy
// Now we are changing the cert checking mechanism to validate the Root CA cert which has a much longer expiry rather than the 3month expiry on the Server cert.
// This should only break either: if hamqsl ever switch to another Certificate Authority OR we are all really old and its past the Root cert expiry date of 01/01/2038...
// Read about Root and Issuing CA chaining here: https://randomnerdtutorials.com/esp8266-nodemcu-https-requests/ Root Certificate method (last method example)
const char starfield_Root [] PROGMEM = R"CERT(-----BEGIN CERTIFICATE-----
MIID3TCCAsWgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBjzELMAkGA1UEBhMCVVMx
EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxJTAjBgNVBAoT
HFN0YXJmaWVsZCBUZWNobm9sb2dpZXMsIEluYy4xMjAwBgNVBAMTKVN0YXJmaWVs
ZCBSb290IENlcnRpZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAw
MFoXDTM3MTIzMTIzNTk1OVowgY8xCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6
b25hMRMwEQYDVQQHEwpTY290dHNkYWxlMSUwIwYDVQQKExxTdGFyZmllbGQgVGVj
aG5vbG9naWVzLCBJbmMuMTIwMAYDVQQDEylTdGFyZmllbGQgUm9vdCBDZXJ0aWZp
Y2F0ZSBBdXRob3JpdHkgLSBHMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC
ggEBAL3twQP89o/8ArFvW59I2Z154qK3A2FWGMNHttfKPTUuiUP3oWmb3ooa/RMg
nLRJdzIpVv257IzdIvpy3Cdhl+72WoTsbhm5iSzchFvVdPtrX8WJpRBSiUZV9Lh1
HOZ/5FSuS/hVclcCGfgXcVnrHigHdMWdSL5stPSksPNkN3mSwOxGXn/hbVNMYq/N
Hwtjuzqd+/x5AJhhdM8mgkBj87JyahkNmcrUDnXMN/uLicFZ8WJ/X7NfZTD4p7dN
dloedl40wOiWVpmKs/B/pM293DIxfJHP4F8R+GuqSVzRmZTRouNjWwl2tVZi4Ut0
HZbUJtQIBFnQmA4O5t78w+wfkPECAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAO
BgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFHwMMh+n2TB/xH1oo2Kooc6rB1snMA0G
CSqGSIb3DQEBCwUAA4IBAQARWfolTwNvlJk7mh+ChTnUdgWUXuEok21iXQnCoKjU
sHU48TRqneSfioYmUeYs0cYtbpUgSpIB7LiKZ3sx4mcujJUDJi5DnUox9g61DLu3
4jd/IroAow57UvtruzvE03lRTs2Q9GcHGcg8RnoNAX3FWOdt5oUwF5okxBDgBPfg
8n/Uqgr/Qh037ZTlZFkSIHc40zI+OIF1lnP6aI+xy84fxez6nH7PfrHxBy22/L/K
pL/QlwVKvOoYKAKQvVR4CSFx09F9HdkWsKlhPdAKACL8x3vLCWRFCztAgfd9fDL1
mMpYjn0q7pBZc2T5NnReJaH1ZgUufzkVqSr7UIuOhWn0
-----END CERTIFICATE-----
)CERT";
X509List cert(starfield_Root);   //
// end root cert additions VK2YU 14Aug2025

// ============ DISPLAY ROUTINES =====================================================

void showClockStatus() {
   const int x = 290, y = 1, w = 28, h = 29, f = 2;  // screen position & size
   int color;
   if (second() % 10) return;                  // update every 10 seconds
   int syncAge = now() - lastNtpUpdateTime();  // how long has it been since last sync?
   if (syncAge < SYNC_MARGINAL)                // GREEN: time is good & in sync
      color = TFT_GREEN;
   else if (syncAge < SYNC_LOST)  // ORANGE: sync is 1-24 hours old
      color = TFT_ORANGE;
   else color = TFT_RED;  // RED: time is stale, over 24 hrs old

   if (WiFi.status() != WL_CONNECTED) {  //
      color = TFT_DARKGREY;              // GRAY: WiFi connection was lost
      WiFi.disconnect();                 // so drop current connection
      // WiFi.begin(WIFI_SSID,WIFI_PWD);             // and attempt to reconnect
      delay(2000);    // allow time for the wifi to disconnect cleanly
      ESP.restart();  // forces the clock to restart and wifimanger will reconnect or revert AP mode
   }

   // begin additions by KD5RXT 20250806
   if (night_mode) {
      switch (color) {
         case TFT_GREEN:
            {
               tft.drawRoundRect(x, y, w, h, 10, TFT_RED);  // show clock status as rounded rectangles
            }
            // no break to drop thru

         case TFT_ORANGE:
            {
               tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 10, TFT_RED);  // show clock status as rounded rectangles
            }
            // no break to drop thru

         case TFT_RED:
            {
               tft.fillRoundRect(x + 2, y + 2, w - 4, h - 4, 10, TFT_RED);  // show clock status as rounded rectangles
            }
            break;
      }
      tft.setTextColor(TFT_BLACK, TFT_RED);
   } else {
      tft.fillRoundRect(x, y, w, h, 10, color);  // show clock status as a color
      tft.setTextColor(TFT_BLACK, color);
   }
   // end additions by KD5RXT 20250806

   tft.drawNumber(-WiFi.RSSI(), x + 6, y + 6, f);  // WiFi strength as a positive value

   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.setTextColor(TIMECOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);
   } else {
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);
   }
}

// begin additions by KD5RXT 20250730
void showBrightSetting(int x, int y) {
   const int f = 2;  // text font
   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.setTextColor(LABEL_FGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);  // set text colors
   } else {
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);  // set text colors
   }
   // end additions by KD5RXT 20250806

   tft.drawString("       ", x, y, f);
   tft.drawString("B:", x, y, f);
   if (brightness == 0) {
      tft.drawString("Auto", x + 14, y, f);  // Modified by VK2ARH to allow for a larger gap between : and Auto for readability
   } else {
      tft.drawNumber(brightness / 32, x + 12, y, f);
   }
}
// end additions by KD5RXT 20250730

/*
 *  Modified by John Price (WA2FZW)
 *
 *    In the original code, this was an empty function. I added code to display either
 *    an 'A' or 'P' to the right of the local time 
 *
 */

void showAMPM(int hr, int x, int y) {
   char ampm;                    // Will be either 'A' or 'P'
   if (hr <= 11)                 // If the hour is 11 or less
      ampm = 'A';                // It's morning
   else                          // Otherwise,
      ampm = 'P';                // It's afternoon
   tft.drawChar(ampm, x, y, 4);  // Show AM/PM indicator
}

void showTime(time_t t, bool hr12, int x, int y) {
   const int f = 7;  // screen font
   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.setTextColor(TIMECOLOR_NIGHT, TFT_BLACK);  // set time color
   } else {
      tft.setTextColor(TIMECOLOR, TFT_BLACK);  // set time color
   }
   int h = hour(t);
   int m = minute(t);
   int s = second(t);                                  // get hours, minutes, and seconds
   if (hr12) {                                         // if using 12hr time format,
      if (DISPLAY_AMPM) showAMPM(h, x + 220, y + 14);  // show AM/PM indicator
      if (h == 0) h = 12;                              // 00:00 becomes 12:00
      if (h > 12) h -= 12;                             // 13:00 becomes 01:00
   }
   if (h < 10) {                           // is hour a single digit?
      if ((!hr12) || (HOUR_LEADING_ZERO))  // 24hr format: always use leading 0
         x += tft.drawChar('0', x, y, f);  // show leading zero for hours
      else {
         tft.setTextColor(TFT_BLACK, TFT_BLACK);  // black on black text
         x += tft.drawChar('8', x, y, f);         // will erase the old digit

         // begin additions by KD5RXT 20250806
         if (night_mode) {
            tft.setTextColor(TIMECOLOR_NIGHT, TFT_BLACK);
         } else {
            tft.setTextColor(TIMECOLOR, TFT_BLACK);
         }
         // end additions by KD5RXT 20250806
      }
   }
   x += tft.drawNumber(h, x, y, f);              // hours
   x += tft.drawChar(':', x, y, f);              // show ":"
   if (m < 10) x += tft.drawChar('0', x, y, f);  // leading zero for minutes
   x += tft.drawNumber(m, x, y, f);              // show minutes
   x += tft.drawChar(':', x, y, f);              // show ":"
   if (s < 10) x += tft.drawChar('0', x, y, f);  // add leading zero for seconds
   x += tft.drawNumber(s, x, y, f);              // show seconds
}

void showDate(time_t t, int x, int y) {
   const int f = 4, yspacing = 30;  // screen font, spacing
   const char* months[] = { "JAN", "FEB", "MAR",
                            "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT",
                            "NOV", "DEC" };
   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.setTextColor(DATECOLOR_NIGHT, TFT_BLACK);
   } else {
      tft.setTextColor(DATECOLOR, TFT_BLACK);
   }
   // end additions by KD5RXT 20250806

   int i = 0, m = month(t), d = day(t);        // get date components
   tft.fillRect(x, y, 50, 60, TFT_BLACK);      // erase previous date
   if (DATE_ABOVE_MONTH) {                     // show date on top -----
      if ((DATE_LEADING_ZERO) && (d < 10))     // do we need a leading zero?
         i = tft.drawNumber(0, x, y, f);       // draw leading zero
      tft.drawNumber(d, x + i, y, f);          // draw date
      y += yspacing;                           // and below it, the month
      tft.drawString(months[m - 1], x, y, f);  // draw month
   } else {                                    // put month on top ----
      tft.drawString(months[m - 1], x, y, f);  // draw month
      y += yspacing;                           // and below it, the date
      if ((DATE_LEADING_ZERO) && (d < 10))     // do we need a leading zero?
         x += tft.drawNumber(0, x, y, f);      // draw leading zero
      tft.drawNumber(d, x, y, f);              // draw date
   }
}

void showTimeZone(int x, int y) {
   const int f = 4;  // text font
   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.setTextColor(LABEL_FGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);  // set text colors
      tft.fillRect(x, y, 80, 28, LABEL_BGCOLOR_NIGHT);             // erase previous TZ
   } else {
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);  // set text colors
      tft.fillRect(x, y, 80, 28, LABEL_BGCOLOR);       // erase previous TZ
   }
   // end additions by KD5RXT 20250806

   if (!useLocalTime)
      tft.drawString("UTC", x, y, f);  // UTC time
   else
      tft.drawString(local.getTimezoneName(), x, y, f);  // show local time zone
}

void showTimeDate(time_t t, time_t oldT, bool hr12, int x, int y) {
   showTime(t, hr12, x, y);                 // display time HH:MM:SS
   if ((!oldT) || (hour(t) != hour(oldT)))  // did hour change?
      showTimeZone(x, y - 42);              // update time zone
   if ((!oldT) || (day(t) != day(oldT)))    // did date change? (Thanks John WA2FZW!)
      showDate(t, x + 250, y);              // update date
}

void updateDisplay() {
   t = now();                                              // check latest time
   if (t != oldT) {                                        // are we in a new second yet?
      lt = local.now();                                    // keep local time current
      useLocalTime = true;                                 // use local timezone
      showTimeDate(lt, oldLt, LOCAL_FORMAT_12HR, 10, 46);  // show new local time
      useLocalTime = false;                                // use UTC timezone
      showTimeDate(t, oldT, UTC_FORMAT_12HR, 10, 172);     // show new UTC time
      showClockStatus();                                   // and clock status
      printTime();                                         // send timestamp to serial port
      oldT = t;
      oldLt = lt;  // remember currently displayed time

      // get solar data first time or on the hour at solar_min
      if (((minute(t) == solar_min) && second(t) == solar_sec) || *solar_cond == 0) {
         // begin additions by KD5RXT 20250806
         if (night_mode) {
            tft.setTextColor(LABEL_BGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);  // set label colors
         } else {
            tft.setTextColor(LABEL_BGCOLOR, LABEL_BGCOLOR);  // set label colors
         }
         // end additions by KD5RXT 20250806

         tft.drawString(solar_cond, 80, 130, 4);  // clear previous data
         getSolarData();
         printTime();

         // begin additions by KD5RXT 20250806
         if (night_mode) {
            tft.setTextColor(LABEL_FGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);  // set label colors
         } else {
            tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);  // set label colors
         }
         // end additions by KD5RXT 20250806

         tft.drawString(solar_cond, 80, 130, 4);
         Serial.println(solar_cond);
      }
   }
}

void newDualScreen() {
   tft.fillScreen(TFT_BLACK);  // start with empty screen

   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.fillRoundRect(0, 0, 319, 32, 10, LABEL_BGCOLOR_NIGHT);     // title bar for local time
      tft.fillRoundRect(0, 126, 319, 32, 10, LABEL_BGCOLOR_NIGHT);   // title bar for UTC
      tft.setTextColor(LABEL_FGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);    // set label colors
      tft.drawCentreString(TITLE, 160, 4, 4);                        // show title at top
      tft.drawRoundRect(0, 0, 319, 110, 10, LABEL_FGCOLOR_NIGHT);    // draw edge around local time
      tft.drawRoundRect(0, 126, 319, 110, 10, LABEL_FGCOLOR_NIGHT);  // draw edge around UTC
   } else {
      tft.fillRoundRect(0, 0, 319, 32, 10, LABEL_BGCOLOR);     // title bar for local time
      tft.fillRoundRect(0, 126, 319, 32, 10, LABEL_BGCOLOR);   // title bar for UTC
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);          // set label colors
      tft.drawCentreString(TITLE, 160, 4, 4);                  // show title at top
      tft.drawRoundRect(0, 0, 319, 110, 10, LABEL_FGCOLOR);    // draw edge around local time
      tft.drawRoundRect(0, 126, 319, 110, 10, LABEL_FGCOLOR);  // draw edge around UTC
   }
   // end additions by KD5RXT 20250806

   // begin additions by KD5RXT 20250730
   showBrightSetting(230, 8);
   // end additions by KD5RXT 20250730
}

void startupScreen() {
   tft.fillScreen(TFT_BLACK);  // start with empty screen

   // begin additions by KD5RXT 20250806
   if (night_mode) {
      tft.fillRoundRect(0, 0, 319, 30, 10, LABEL_BGCOLOR_NIGHT);   // title bar
      tft.drawRoundRect(0, 0, 319, 239, 10, LABEL_FGCOLOR_NIGHT);  // draw edge screen
      tft.setTextColor(LABEL_FGCOLOR_NIGHT, LABEL_BGCOLOR_NIGHT);  // set label colors
      tft.drawCentreString(WIFISTART, 160, 2, 4);                  // show sketch title on screen
      tft.setTextColor(LABEL_FGCOLOR_NIGHT, TFT_BLACK);            // set text color
   } else {
      tft.fillRoundRect(0, 0, 319, 30, 10, LABEL_BGCOLOR);   // title bar
      tft.drawRoundRect(0, 0, 319, 239, 10, LABEL_FGCOLOR);  // draw edge screen
      tft.setTextColor(LABEL_FGCOLOR, LABEL_BGCOLOR);        // set label colors
      tft.drawCentreString(WIFISTART, 160, 2, 4);            // show sketch title on screen
      tft.setTextColor(LABEL_FGCOLOR, TFT_BLACK);            // set text color
   }
   // end additions by KD5RXT 20250806
}


// ============ MISC ROUTINES =====================================================

/*void saveConfigCallback () {                      // used for saving config items if set
  shouldSaveConfig = true;
}
*/
void showConnectionProgress() {
   int elapsed = 0;
   tft.drawString("WiFi connected at: ", 5, 50, 4);
   while (WiFi.status() != WL_CONNECTED) {    // while waiting for connection
      tft.drawNumber(elapsed++, 230, 50, 4);  // show we are trying!
      delay(1000);
   }
   tft.drawString("IP = " +  // connected to LAN now
                    WiFi.localIP().toString(),
                  5, 80, 4);  // so show IP address
   elapsed = 0;
   tft.drawString("Waiting for NTP", 5, 140, 4);  // Now get NTP info
   while (timeStatus() != timeSet) {              // wait until time retrieved
      events();                                   // allow ezTime to work
      tft.drawNumber(elapsed++, 230, 140, 4);     // show we are trying
      delay(1000);
   }
}

void printTime() {             // print time to serial port
   if (!PRINTED_TIME) return;  // option 0: dont print
   Serial.print("TIME: ");
   if (PRINTED_TIME == 1)
      Serial.println(dateTime(TIME_FORMAT));  // option 1: print UTC time
   else
      Serial.println(local.dateTime(TIME_FORMAT));  // option 2: print local time
}

void blink(int count = 1) {           // diagnostic LED blink
   pinMode(LED_PIN, OUTPUT);          // make sure pin is an output
   for (int i = 0; i < count; i++) {  // blink counter
      digitalWrite(LED_PIN, 0);       // turn LED on
      delay(200);                     // for 0.2s
      digitalWrite(LED_PIN, 1);       // and then turn LED off
      delay(200);                     // for 0.2s
   }
   pinMode(LED_PIN, INPUT);  // works for both Vcc & Gnd LEDs.
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
   WiFiClientSecure client;         // used for the TLS connection VK2YU 14Aug2025
   client.setTrustAnchors(&cert);   // set the root certificate as the trust anchor for the HTTP client connection VK2YU 14Aug2025
   HTTPClient https;                // establish a secure http connection
   //client.setFingerprint(fingerprint);      // set the HTTPS fingerprint (not required since root cert trust)
   https.begin(client, SW_URL);              // open the URL connection
   int httpResponseCode = https.GET();       // get the response code
   sprintf(solar_cond, "*** no data ***");  // init the report in case if fails
   for (int i = 0; i < 5; ++i) {            // try up to 5 times
      if (httpResponseCode > 0) {           // if we got a response try to use it
         Serial.print("HTTPS Response code: ");
         Serial.println(httpResponseCode);
         String payload = https.getString();  // get the XML
         Serial.println(payload);
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
   https.end();
}

// begin additions by KD5RXT 20250731
void updateBrightness(void) {
   static int previous_bright_level;
   int bright_level;

   if (brightness == 0) {
      bright_level = (previous_bright_level + (analogRead(LIGHT_SENSOR_PIN) / 4)) / 2;

      if (bright_level == 0) {
         previous_bright_level = 0;
         bright_level = 1;
      }

      analogWrite(LED_BACKLIGHT_PIN, bright_level);
      previous_bright_level = bright_level;
   } else {
      analogWrite(LED_BACKLIGHT_PIN, brightness);
   }
}
// end additions by KD5RXT 20250731

// ============ MAIN PROGRAM ===================================================

void setup() {
   // begin additions by KD5RXT 20250731
   pinMode(LED_BACKLIGHT_PIN, OUTPUT);
   analogWrite(LED_BACKLIGHT_PIN, brightness);
   // end additions by KD5RXT 20250731

   tft.init();                           // initialize LCD screen object
   tft.setRotation(SCREEN_ORIENTATION);  // landscape screen orientation
   startupScreen();                      // show Wifi is starting
   blink(3);                             // show sketch is starting
   Serial.begin(BAUDRATE);               // open serial port
   setDebug(DEBUGLEVEL);                 // enable NTP debug info
   setServer(NTP_SERVER);                // set NTP server
   // begin additions by KD5RXT 20250731
   // tft.fillScreen(TFT_BLACK);         // start with empty screen
   // end additions by KD5RXT 20250731
   local.setPosix(TZ_RULE);  // estab. local TZ by rule
   //WifiManagerParameter customCallSign("callSign", "Enter your Callsign: ", callSign, 8);  //New object to collect -Callsign of upto 8 characters

   WiFiManager wifiManager;  // starts the wifimanager. To capture WiFi details if not already
   //wifiManager.setSaveConfigCallback(saveConfigCallback);  // adds the process to capture and save additional variables
   //wifiManager.addParameter(&customCallSign);     // adds the custom callsign parameter to collect

   wifiManager.autoConnect("NTPClockAP");  // set the name of the AccessPoint for WifiManager
   server.begin();                         // starts a simple webserver to capture user input for wifi
                                           //strcpy(callSign, customCallSign.getValue());    //copy values to memory from Wifimanger
                                           // if (shouldSaveConfig) {
                                           // saveConfig();
                                           // }

   //TITLE = callSign();                            // to be used later -VK2YU
   //WiFi.begin(WIFI_SSID, WIFI_PWD);               // start WiFi (redundant since wifimanager)
   showConnectionProgress();  // WiFi and NTP may take time
   delay(2000);
   tft.fillScreen(TFT_BLACK);         // start with empty screen
   // The next line simply sets the ESP device internal time against an NTP source to ensure the SSL cert check passes in the connection to hamqsl. 
   // SSL Certificate checking is dependent on the server/client device time syncs being accurate. 
   // It has no bearing on the displayed "NTPClock" "time as this is handled in the original time function. VK2YU 14Aug2025
   configTime(10 * 3600, 0, "pool.ntp.org");    
   newDualScreen();                   // show title & labels
   *solar_cond = 0;                   // initialize the conition buffer to be a null string
   solar_sec = second(local.now());   // we'll poll on this second as pseudo random
   solar_min = (solar_sec) % 5 + 32;  // we'll poll some time in the first 5 minutes
                                      // or so after the half hour.
                                      // This is to spread out other clocks for load
                                      // balancing.
   // begin additions by KD5RXT 20250731
   SW2.begin();
   // end additions by KD5RXT 20250731
}

void loop() {
   events();         // get periodic NTP updates
   updateDisplay();  // update clock every second


   // begin additions by KD5RXT 20250730
   switch (SW2_state) {
      case BUTTON_MODE_IDLE:
         {
            if (SW2.pressed()) {
               SW2_state = BUTTON_MODE_PRESSED;
// begin additions by KD5RXT 20250806
            } else {
               press_duration = 0;
            }
// end additions by KD5RXT 20250806
         }
         break;

      case BUTTON_MODE_PRESSED:
         {
            // begin additions by KD5RXT 20250806
            if (!SW2.read()) {
               if ((millis() - check_button_hold_interval) > CHECK_BUTTON_LONG_PRESS_INTERVAL_IN_MILLIS) {
                  check_button_hold_interval = millis();
                  if (press_duration < LONG_PRESS_DURATION_IN_SECONDS) {
                     press_duration++;
                  }
               }
            }
            // end additions by KD5RXT 20250806

            if (SW2.released()) {
               SW2_state = BUTTON_MODE_ACTIVATED;
            }
         }
         break;

      case BUTTON_MODE_ACTIVATED:
         {
            SW2_state = BUTTON_MODE_IDLE;

            // begin additions by KD5RXT 20250806
            if (press_duration >= LONG_PRESS_DURATION_IN_SECONDS) {
               night_mode = !(night_mode);
               newDualScreen();
               t = now();                                       // check latest time
               lt = local.now();                                // keep local time current
               useLocalTime = true;                             // use local timezone
               showTimeDate(lt, 0, LOCAL_FORMAT_12HR, 10, 46);  // show new local time
               useLocalTime = false;                            // use UTC timezone
               showTimeDate(t, 0, UTC_FORMAT_12HR, 10, 172);    // show new UTC time
               showClockStatus();                               // and clock status
               sprintf(solar_cond, "");                         // init the report in case if fails
               printTime();                                     // send timestamp to serial port
               oldT = t;
               oldLt = lt;  // remember currently displayed time
               press_duration = 0;
            } else {
               brightness += 32;

               showBrightSetting(230, 8);

               // begin additions by KD5RXT 20250731
               updateBrightness();
               // end additions by KD5RXT 20250731
            }

            press_duration = 0;
            // end additions by KD5RXT 20250806
         }
         break;
   }

   if ((millis() - check_light_sensor_interval) > CHECK_LIGHT_SENSOR_INTERVAL_IN_MILLIS) {
      check_light_sensor_interval = millis();

      // begin additions by KD5RXT 20250731
      updateBrightness();
      // end additions by KD5RXT 20250731
   }
   // end additions by KD5RXT 20250730
}
