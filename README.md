
# C8 Clutch Pressure Controller

Have you run into an issue where you need more clutch pressure?\
Have you tried aftermarket clutch pressure controllers?\
Did they fail you once you turned them up to advertised maximum pressure?\
Did you really need to spend all that money?\
Do you want to try something before HPTuners releases support for C8 TCM? \
Are you electronically competent ?

Well this writeup is for you...  I will show you how to build it, and provide you with code which you can use as starting point.


**Hardware**\
ESP32 S2       <<<< i will be using ESP32-S2-Saola-1, please dont use AMAZON knockoffs\
6x 2k Resistors\
6x 1k Resistors\
MCP4911-E/P\
MCP4912-E/P\
K78L05-500R3   <<< or somekind of 5V regulator


**Software**\
Visual Studio Code\
PlatformIO


**How does it work**\
We will take a pressure signal from Base, Odd and Even sensors located in Transmission.  Read those pressures and also feed them into MCPs.  At this point, our code will make a decision if signal can be passed along into TCM or if it needs to be modified in MCP before its passed along to TCM.


**Important**\
All signals in question are 5V, but ESP32 operates on 3.3V and it is not 5V tolerant.  That is where resistors come in play.\
We will read values before and after MCP.  This will allow us to log, compare and debug.\


Here is pinout of ESP32 i will be using
![ESP32](esp32-s2_saola1-pinout.jpg)
