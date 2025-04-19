This repository holds the files used for the Bruce Hall W8BH NTP Clock. The clock displays both local and UTC time derived and reqularly updated from an internet time server via wifi. The build project took Bruce's original BN33 PCB and added a top and bottom cover to house the clock board. Gerbers are available for these covers with or without the Manly Warringah Radio Society (MWRS) logo seen in the picture. 

![image](https://github.com/user-attachments/assets/14b7e2cb-526c-4650-acd3-af152780c68b)

Documentation from W8BH and the CalQRP group are also contained in this repository as a 'one stop' shop to secure the information and code needed for the build. Details of the software to run the dual time clock as well as how to configure the code with your callsign, network SSID, password and location are contained in the documentation. 

Bruce's website and additional information can be found here: http://w8bh.net/

The clock can be configured for any location by inserting the appropriate string in the code. This string identifies the offset from UTC and the day and times of changes to daylight savings (eg:2nd sunday of April at 2:00am). Details of where to find the string for your location is covered in the documentation. 

The clock takes about an hour to build and is a fun little project which delivers a great practical addition to the shack. 

![image](https://github.com/user-attachments/assets/59c70f6c-8328-487c-b480-25ff34069805)

Whilst the clock can be powered directly with a USB-C connection to the onboard ESP8266 module, Bruce's project made use of a buck converter to support a wide range of input voltages from 7-15v, making it ideal to be powered from a 13.8v power supply in the shack.

Full details of the capabilty of the clock, how to build and configure it are contained in the documentation. Further updates will be posted here should any update to the software or gerber files become availble. The software is open source so feel free to improve or modify the code. Options could include an ability to input WiFi network ID, password and callsign, selecting additional information to be displayed eg: local or solar weather, providing a dimming function etc.

The W8BH PCB enables the use of either a 2.2", 2.8" or 3.2" screen. This build uses the 2.8" LCD screen as it was chosen as the perceived best price/screen size option, although it does requires the removal of some PCB material around the mounting holes of the 2.8" display board to provide clearance for the standoffs to mount the display in the 'PCB sandwich' housing. This trimming of the display PCB can be done with confidence knowing that there are no tracks around the mounting holes. 

The holes on a 3.2" LCD display align perfectly with the W8BH PCB holes and the use of a 2.2" board will allow the mounting spacers to be used without any modification to the 2.2" board, but in both these cases a different top cover would be required to align with the display. My recommendation - stick with the 2.8" screen if you wish to use the top cover.

The standoffs between the PCB and Bottom cover are 15mm and 16mm (15mm + 1mm Plastic washer)between the Top Cover and the PCB. The build could be made thinner if components were soldered directly to the main PCB, but if you are happy to do that, simply adjust the size of the spacers between the boards.

![image](https://github.com/user-attachments/assets/685b4c6e-ecf7-4b40-9a97-8eb329498cef)

Simon VK2YU designed a great 3D printed housing for the NTP clock using the top cover shown in this project. The link to the .stl files in Thingiverse can be found here: https://www.thingiverse.com/thing:7013343  Thanks Simon :) 

![image](https://github.com/user-attachments/assets/fe04f1a4-b68c-4498-a936-82e995905eb8)

