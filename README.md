This repository holds the files used for the Bruce Hall W8BH NTP Clock. The clock displays both local and UTC time derived and reqularly updated from an internet time server via wifi. The build project took Bruce's original PCB and added a top and bottom cover to house the clock board. Gerbers are available for these covers with or without the Manly Warringah Radio Society (MWRS) logo seen in the picture. 

![image](https://github.com/user-attachments/assets/14b7e2cb-526c-4650-acd3-af152780c68b)

Documentation from W8BH and the CalQRP group are also contained in this repository as a 'one stop' shop to secure the information and code needed for the build. Details of the software to run the dual time clock as well as how to configure the code with your callsign, network SSID, password and location are contained in the documentation. 

Bruce's website and additional information can be found here: http://w8bh.net/

The clock can be configured for any location by inserting the appropriate string in the code. This string identifies the offset from UTC and the day and times of changes to daylight savings (eg:2nd sunday of April at 2:00am). Details of where to find the string for your location is covered in the documentation. 

The clock takes about an hour to build and is a fun little project which delivers a great practical addition to the shack. 

![image](https://github.com/user-attachments/assets/59c70f6c-8328-487c-b480-25ff34069805)

Whilst the clock can be powered directly with a USB-C connection to the onboard ESP8266 module, Bruce's project made use of a buck converter to support a wide range of input voltages from 7-15v, making it ideal to be powered from a 13.8v power supply in the shack.

Full details of the capabilty of the radio, how to build and configure it are contained in the documentation. Further updates will be posted here should any update to the software or gerber files become availble. The software is open source so feel free to improve or modify the code. Options could include using the Colour LED display's touch featue to input network ID and password, selecting additional information to be displayed eg: local or solar weather, providing a dimming function etc.

The W8BH PCB enables the use of either a 2.2", 2.8" or 3.2" screen. This build uses the 2.8" LCD screen, which requires a little bit of filing/removal around the mounting holes of the 2.8" LCD board to provide the necessary clearance for the standoffs to mount the display in the 'PCB sandwich' shown in the photos. This filing and/or cutting/shaping of the display board can be easily undertaken as the board does not contain any tracks around the mounting holes. 

The 2.8" screen was chosen as the perceived best price/screen size option. The holes on a 3.2" LCD display align perfectly with the W8BH PCB holes and the use of a 2.2" board will allow the mounting spacers to be used without any modification to the 2.2" board, but in both these cases a different top cover would be required to align with the display. My recommendation - stick with the 2.8" screen if you wish to use the top cover.

The standoffs between the PCB and Bottom cover are 15mm and 16mm (15mm + 1mm Plastic washer)between the Top Cover and the PCB. The build could be made thinner if components were soldered directly to the main PCB, but this would provide difficulties for subesquent replacement if required.

![image](https://github.com/user-attachments/assets/685b4c6e-ecf7-4b40-9a97-8eb329498cef)

Simon VK2YU designed a great 3D printed housing for the NTP clock using the top cover shown here. The link to the .stl files in Thingiverse will be listed shortly.

![image](https://github.com/user-attachments/assets/fe04f1a4-b68c-4498-a936-82e995905eb8)

