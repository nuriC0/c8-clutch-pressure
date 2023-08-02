
# C8 Clutch Pressure Controller

Have you encountered a situation where you require additional clutch pressure?\
Have you attempted using aftermarket clutch pressure controllers without success?\
Do you want to try something before HPTuners releases support for C8 TCM? \
Are you electronically competent ?

Well this writeup is for you...\
I will guide you through the process of constructing a controller and offer you code that can serve as a starting point.

I will **not**  show you how to
* Get to TCM (there are plenty of directions out there)
* How to depin TCM Connector A (96pin)
* How to tap into 12V at TCM
* Where to place your newly built device


**How does it work**\
The signals from pressure sensors in the transmissions are intercepted, and subsequently, these pressures are read and fed into MCP-DACs. At this stage, our code will determine whether the signal can be directly transmitted to the TCM or if it needs to be adjusted within the MCP-DAC before being passed on to the TCM.

We will be handling three pressure sensors: one for the base transmission pressure and two for the clutch pressures - one for odd gears and the other for even gears.


**Hardware**
* ESP32 S2        <<<< i will be using ESP32-S2-Saola-1, please dont use AMAZON knockoffs
* 6x 2k Resistors
* 6x 1k Resistors
* MCP4911-E/P
* MCP4912-E/P
* K78L05-500R3    <<< or somekind of 5V regulator
* 3x TE 1670146-1 <<< pins for TCM connector (will have to cut off insolation part)
* Prototyping solderable breadboard, i use PTSolns off Amazon

*Everything on above list can be purchased on digikey.com*

**Software**
* Visual Studio Code
* PlatformIO


**Important**
* All signals in question are 5V, but ESP32 operates on 3.3V and it is not 5V tolerant.  That is where resistors come in play.  I will take all signals and run them thru voltage dividers\
* Code will read values **before** and **after** MCP-DAC.  This will allow us to log, compare and debug.\
* Following 3 pins will need to be depined on TCM Connector A (96 pin).  Pin 1 (green/blue), Pin 3 (yellow/black) and Pin 5 (white/brown).\
* I will refer to Pin 1 signal as **EVEN**, Pin 3 as **BASE** and Pin 5 as **ODD**.\
* The TCM periodically wakes up to check on sensors, which makes powering on with the key not feasible. To address this, we have a few options to consider. The first option is to keep the device always on, consuming 27mA of power. The second option is to put the device in a half-sleep mode, which consumes 17mA of power. The third option is to place the device into DEEP sleep mode if the signals remain at 0 for 5 seconds, and we can use the Up Shift signal to wake it up, utilizing the RTC and consuming 7mA of power.


**ESP32 S2 PINOUT used in this document and code**

![ESP32](esp32-s2_saola1-pinout.jpg)

I will be using following GPIO

GPIO1 = BASE Input\
GPIO2 = ODD Input\
GPIO3 = EVEN Input

GPIO6 = BASE Output\
GPIO7 = ODD Output\
GPIO8 = EVEN Output


**Basic Diagram**

![ESP32](cpc_diagram.png)


$${\color{red}Depending \space on \space your \space wiring \space and \space rest \space of \space setup, \space you \space might \space need \space to \space add \space some \space filtering \space on \space signal \space side}$$



**Understanding MCPs**

This information is also available on ICs datasheets. **Important** I have also noted ESP *GPIO* to be used with pins.  This is how you 'wire' it.


![MCP49x1](MCP49x1.png)

![MCP49x2](MCP49x2.png)


**Voltage Dividers**

![Voltage Dividers](board/VoltageDivider.png)


**5 Volt Regulator**

![5 Volt Regulator](board/5VoltRegulator.png)


# Basic Board Layout

In this pic we can see 5Volt jump wire from ESP to ICs.  Also 2K Resistors presoldered before we put ESP32 on board. (ignore hole 10 resistors)

![Step1](board/pic1.jpg)


ESP32 and MCP-DACs on board

![Step2](board/pic2.jpg)


SPI wires, 1k resistors with jumper wires to GPIO1/2/3, power wires where regulator would go.  I also ran other wires on other side of board, to balance out mess of wires.

![Step3](board/pic3.jpg)



**At this point you should have an idea on what needs to be soldered and to what points.  Its called out above**




## Apendix


1.0 - Reading logs

Blue - This is what happens if you start to modify BASE pressure while clutch pressure was already modified.  Clutch and Base should always go together

Yellow - This is what happens if your trigger/gate pressure is set to LOW.  In this case, i let go off the gas but signal was still being modified. Variable *_clutchGate* is to LOW in code.

![Apendix1](board/log_example1.jpg)




2.0 - Simular devices using Arduino Nano 




![Apendix2](board/apendix1.jpg)

![Apendix2](board/apendix2.jpg)
