This repository holds the files used for a build of the Bruce Hall W8BH NTP Clock. The build project took the original PCB and added a top and bottom cover to house the clock board. Gerbers are available for these covers without the Logo or with the Manly Warringah Radio Society (MWRS) logo seen in the picture. 

Documentation from W8BH and the CalQRP group are also contained in this repository as a 'one stop' shop to secure the information and code needed for the build. Details of the software to run the dual time clock as well as how to configure the code with your callsign, network SSID and password are contained in the documentation. 

Bruce's website and additional information can be found here: http://w8bh.net/

![image](https://github.com/user-attachments/assets/14b7e2cb-526c-4650-acd3-af152780c68b)

The clock can be confired for any location within the world, by inserting the appropriate string for the desired time zone in the code which identifies the offset from UTC and the dates and times of the changes to daylight savings. On startup and at regular intervals, the clock receives time synchronisation from the designated internet time server via WiFi, calculates local time which is automatically adjusted for daylight savings. It's a quick build, fun little project and a great practical addition to the shack.

![image](https://github.com/user-attachments/assets/59c70f6c-8328-487c-b480-25ff34069805)

Whilst the clock can be powered directly with a USB-C connection to the onboard ESP8266 module, Bruce's project made use of a buck converter to support a wide range of input voltages from 7-15v, making it ideal to be powered from a 13.8v power supply in the shack.

Full details of the capabilty of the radio, how to build and configure it are contained in the documentation. Gerber files have been provided for Bruce's PCB and the top and bottom cover used by the Manly Warringah Radio Society. Further updates will be posted here should any update to the software of form factor become availble. The software is open source so feel free to improve or modify the code. Options could include using the Colour LED display's touch featue to input network ID and password, selecting additional information to be displayed eg: local or solar weather, providing a dimming function etc.

The W8BH PCB enables the use of either a 2.2", 2.8" or 3.2" screen. This build uses the 2.8" LCD screen, which requires a little bit of filing to the edges around the mounting holes of the 2.8" LCD board to provide the necessary clearance for the standoffs to mount the display in the 'PCB sandwich' shown in the photos. This filing and/or cutting/shaping of the display board can be easily undertaken as the board does not contain any tracks around the mounting holes. The 2.8" screen was chosen as the perceived best price/screen size option. The holes on a 3.2" LCD display align perfectly with the W8BH board holes and the use of a 2.2" board will allow the mounting spacers to be used without any modification to the 2.2" board, but in both these cases a different top cover would be required to align with the display. My recommendation - stick with the 2.8" screen if you wish to use the top cover.

Further updates may produce a new main board and relocate the holes to ensure that the 2.8" screen does not require any modification to allow use of the standard W8BH main board. Any modified board will be posted here should it be designed. 

The standoffs between the PCB and Bottom cover are 15mm and 16mm (15mm + 1mm Plastic washer)between the Top Cover and the PCB. The build could be made thinner if the ESP8266 and buck converter were soldered directly to the main PCB, but this would provide difficulties for subesquent replacement if required.


![image](https://github.com/user-attachments/assets/685b4c6e-ecf7-4b40-9a97-8eb329498cef)
