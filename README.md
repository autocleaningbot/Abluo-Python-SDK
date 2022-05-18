# Abluo-Python-SDK
## Arduino Driver
@file abluo_tools.ino
@brief Abluo Tool Control Driver for Arduino Mega

* Takes in Serial Input of the form <toolId,status,direction,speed>
* Updates Data Model to store toolState
* Executes Software PWM to control multiple motors handling tools

```
Current Tools   |           Valid Params            |                   Description                         |
--------------------------------------------------------------------------------------------------------------
1. Brush Servo   : <toolId,status>                   | status: 1 - Lock, 0: Unlock
2. Brush DC      : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
3. Water Pump DC : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
4. Soap Pump DC  : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
```
 
@author Rishab Patwari (patwariri@gmail.com)<br>
@references: https://www.baldengineer.com/software-pwm-with-millis.html
