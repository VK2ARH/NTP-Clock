**Updated 28th December 2025**

Latest version of sofware for ESP8266 board v2.5.35 uploaded which includes support for BME280 Pressure, Humidity and Temperature display. 

**Updated 7th December 2025**

Latest version of sofware for ESP8266 board v2.5.32 uploaded - only minor changes from previous release.


**Update 2nd December 2025**

Note: the current version of the softare v2.5 currently only operates on the ESP8266 processor - we are working to get it running on the ESP32, but that may take a little while - the firmare will be updated here when completed.

**Updated 29th November 2025**

The latest version of the software v2.5 has merged the functionality developed by Robert AI6P and John WA2FZW with the functionality used by the Manly Warringah Radio Society (MWRS) Build project firmware (Contributions by Mark KD5RXT, Simon VK2YU and Richard VK2ARH) into a single instance which now provides the following additional features over the original NTP Clock:
   -   WiFi Manager
   -   Enhanced status displays during WiFi connection
   -   EEPROM storage of Day/Night Mode settings
   -   Day/Night Mode displays
   -   Dimming capability
   -   WiFi Manager Reset Switch
   -   Ability to display multiple time zones
   -   Defining a list of networks which to search for a wifi network during startup
   -   Enhanced solar indicies display capabilities
   -   WiFi Signal loss status indicator with robust background restarting process
     
A video covering these updates can be found here: 

https://youtu.be/-XrX0bBR6TU


**Update 31st October 2025**

The latest version of the software is now V2.3. This release includes a permanant solar data server certificate, dimming and night mode functions and use of WiFi Manager to enter your WiFi network credentials. Thanks to Mark KD5RXT anmd Simon VK2YU for their support and contribution to the development of this code, along with the work done by Robert AI6P and John WA2FZM.

The Original NTP Clock software developed by Bruce Hall W8BH has been enhanced as follows:
- Robert Kincaid AI6P added solar indices to the UTC display header after downloading solar data from the ‘hamsql.com’ website run by Paul Herrman N0NBH. These are updated approximately every 30 minutes with data pulled down from Paul’s solar weather server.
- Simon VK2YU added WiFi Manager to better manage Wi-Fi connections and allow screen-based input from your cell phone rather than hard coding the wifi credentials into the NTP Clock firmware.
- Mark KD5RXT developed code to support the dimming functionality along with the ‘Night Mode’ display.
- Robert AI6P and John WA2FZM have further refined the code used by the CalQRP group which provides an extended range of sola data display, restructured the code and provided a permanent server certificate to access the solar data (this has also been incorporated into the MWRS code), but does not yet include the dimming, night mode or WiFi manager functions. Their code can be found here together with how to use the code with an ESP32 and a CYD (Cheap Yellow Display): 
   http://github.com/WA2FZW/W8BH-NTP-Clock-Revisited-by-WA2FZW-and-AI69

As software continues to develop it is likely that features of both versions of the software may be incorporated into a single version, but this is yet to be undertaken. Discussion in the rest of this document relates to the MWRS version of the software.
The software will need to be configured with your callsign (or any other header you wish to display) and the local time zone if you wish to operate the clock in any location other that Australian Eastern Standard Time (Sydney).
Details of how to change the code to support other time zones can be found in the CalQRP_Club-NTP Clock_ReadMeFirst 2025-04-04.pdf document and a full list of global time zone ‘code string’ can be found here:
https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv


There are two .ino files here for use with the NTP Clock - the first NTP_DualClock.ino is the original firmware provided by Bruce Hall W8BH. The second is the modified firmware which includes to solar indicies modification by Robert Kincaid AI6P together with the WiFi Manager functionality added by Simon VK2YU, and the dimming and night mode function added by Mark KD5RXT. I recommend that you use the second .ino file NTP_Dual_Clock_MWRS_V2.3.ino . This version will create a wireless access point when there are no WiFi credentials in the NTP clock that can be used - there will be a blank screed after about 15 seconds if the NTP clock cannot find a WiFi network.

Upon first power up, if no WiFi credentials are stored in the D1 mini (or the stored network is not available) you need to enter new Wifi Credentials using the WiFi Manager capability. After about 15-20 seconds, you will be looking at a blank screen on the NTP Clock. 
-  Open up your mobile phone and look for a WiFi access point “NTPClockAP”. Once connected to the AP the WiFiManager screen should be displayed - select Configure WiFi (1)
-  Identify the WiFi network SSID that you wish to connect to by tapping on the listed network (2), then enter the password for that network (3). Hit the SAVE button (4) and the NTP Clock will restart. The clock should now connect to your chosen WiFi network and commence operation. The information screen (5) can be displayed by selecting the Info button on the first WiFiManager screen

   <img width="646" height="450" alt="image" src="https://github.com/user-attachments/assets/85f59406-3152-4f09-b7ff-def83a2cebbd" />
      			
**Update 31 July 2025**

Added updated software which incorporates Display dimming control which can be either done manually or using the LDR incorporated into the MWRS design. A video which shows the feature in operation can be found here:

https://youtu.be/ylc6PuMD5ms?si=0apfq-GtRLBQQR25

<img width="1627" height="450" alt="image" src="https://github.com/user-attachments/assets/32440aa5-83af-4986-93cc-b84dd85f8835" />

**Update 1 August 2025**

Minor modification to screen dimming function to correct blank screen when on Auto brightness setting.

**Update 11th August 2025**

New version added which supports night mode (MWRS V2_2) - hold down SW2 for > 3 seconds to toggle between night / day mode. Many thanks for Mark KD5RXT for developing this functionality. Photo shows both Day and Night version (in real life the images on the screen are much sharper than those shown in the photo), clocks are also mounted in the 3D printed housing developed by Simon VK2YU. A video demonstrating switching between day and night mode can be found here: https://youtu.be/ylc6PuMD5ms?si=G7lI8I07H4Umw_Xh

<img width="2754" height="1052" alt="image" src="https://github.com/user-attachments/assets/1be58430-7729-4189-85fc-d839d59ca39e" />


Update 22nd AUGUST 2025

This version of the software (MWRS V2_3) incorporates a longer HAMSQL server certificate to display the solar data for a longer period before a new certificate is required. When the NTP clock is unable to display solar data a text string showing *** no data *** appears on the screen as shown below. This can be a result of server outage or expiration of the server certificate.

<img width="1692" height="1108" alt="image" src="https://github.com/user-attachments/assets/2d7e16b1-084a-4456-896a-eac03bc316d4" />
