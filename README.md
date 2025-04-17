This repository was holds the files used for a build of the Bruce Hall W8BH NTP Clock. The build project took the original PCB used by W8BH and added a top and bottom cover to house the clock board. Gerbers are available for these covers without the Logo or with the Manly Warringah Radio Society (MWRS) logo. Documentation from W8BH and the CalQRP group are also contained in this repository as a 'one stop' shop to secure the documentation for the build.

Bruce's website and additional information can be found here: http://w8bh.net/

![image](https://github.com/user-attachments/assets/14b7e2cb-526c-4650-acd3-af152780c68b)

The clock can be confired for any location within the world, recveives time synchronisation from the internet via WiFi. Local time which is automatically adjusted from daylight savings together with UTC. It is a fun little project and a great addition to the shack.

![image](https://github.com/user-attachments/assets/59c70f6c-8328-487c-b480-25ff34069805)

Whilst the clock can be powered directly with a USB-C connection to the onboard ESP8266 module, Bruce's project made use of a buck converter to support a wide range of input voltages from 7-15v, making it ideal to be powered from a 13.8v power supply in the shack.

Full details of the capabilty of the radio, how to build and configure it are contained in the documentation. Gerber files have been provided for Bruce's PCB and the top and bottom cover used by the Manly Warringah Radio Society. Further updates will be posted should any update to the software of form factor become availble. 

This build uses the 2.8" LCD screen, which requires a little bit of filing to the edges around the mounting holes of the 2.8" LCD board in order to provide the necessary clearance to mount the display in the 'PCB sandwich' shown in the photos. This can be easily undertaken as the LCD board does not contain any tracks around the mounting holes. The holes on a a 3.2" LCD display align perfectly with the W8BH board holes even though pin connections are also provided for the 2.4" and 2.8" LCD displays on the PCB. A different top cover would be required to align with the display if you used anything other than the 2.8" display. The 2.8" screen was choses and the best price/screen size option. 

Further updates may produce a new main board and relocate the holes to ensure that the 2.8" screen does not require any modification to allow use of the standard W8BH main board. Any modified board will be posted here should it be designed. 

The standoffs between the PCB and Bottom cover are 15mm and 16mm (15mm + 1mm Plastic washer)between the Top Cover and the PCB. The build could be made thinner if the ESP8266 and buck converter were soldered directly to the main PCB, but this wouold provide difficulties for subesquent replacement if required.


![image](https://github.com/user-attachments/assets/685b4c6e-ecf7-4b40-9a97-8eb329498cef)
