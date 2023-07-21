# c8-clutch-pressure
C8 Clutch Pressure Controller

Have you run into an issue where you need more clutch pressure?
Have you tried aftermarket clutch pressure controllers?
Did they fail you once you turned them up to advertised maximum pressure?
Did you really need to spend all that money?
Do you want to try something before HPTuners releases support for C8 TCM? 
Are you electronically competent ?

Well this writeup is for you...  I will show you how to build it, and provide you with code so you can go out and have fun.

Hardware:
ESP32 S2       <<<< i will be using ESP32-S2-Saola-1, please dont use AMAZON knockoffs
6x 2k Resistors
6x 1k Resistors
MCP4911-E/P
MCP4912-E/P
K78L05-500R3   <<< or somekind of 5V regulator

Software
Visual Studio Code
PlatformIO

Here is pinout of ESP32 i will beusing
![ESP32](esp32-s2_saola1-pinout.jpg)
