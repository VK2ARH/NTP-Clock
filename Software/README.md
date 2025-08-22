This subdirectory holds the code for the ES8266 to operate the NTP Clock. The Arduino IDE needs to have the apprporiate library loaded in order to upload the firware to the ESP8266. Details of how to upload this library can be found in the Documentation subdirectory of this repository, and should be read inconjunction with the library details documented in the code.

There are two .ino files here for use with the NTP Clock - the first NTP_DualClock.ino is the original firmware provided by Bruce Hall W8BH. The second is the modified firmware which includes to solar indicies modification by Robert Kincaid AI6P together with the WiFi Manager functionality added by Simon VK2YU. I recommend that you use the second .ino file NTP_Dual_Clock_VK2YU_V1.ino . This version will create a wireless access point when there are no WiFi credentials in the NTP clock that can be used - there will be a blank screed after about 15 seconds if the NTP clock cannot find a WiFi network.

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
