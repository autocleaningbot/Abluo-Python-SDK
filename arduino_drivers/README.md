# Abluo-Python-SDK
## Overview
This repo consists of the following directories:

Directory | Package | Description
---| --- | --- |
arduino_drivers | abluo_tools_driver | driver to control tools selection on abluo
arduino_drivers | abluo_wheels_driver | driver to command wheels
abluo_sdk | abluo_api | initialises abluo_tools and abluo_wheels api and connects to logitech-controller to receive inputs
logitech-controller | controller.py | script to call abluo_api and respond to inputs from logitech F710 gamepad

## Startup 
1. To start controller and connect to abluo_api automatically, simply run `start_controller.sh`
2. To start only the abluo_api, install the package and import abluo_api

## Testing
1. Abluo Tools: Call abluo_tools.testTools()
2. Abluo Wheels: Call abluo_wheels.testMovement()

## Arduino Drivers
### Abluo Tools Driver
```
@file abluo_tools_driver.ino<br>
@brief Abluo Tool Control Driver for Arduino Mega
@author Rishab Patwari (patwariri@gmail.com)<br>
@references: https://www.baldengineer.com/software-pwm-with-millis.html

* Takes in Serial Input of the form <toolId,status,direction,speed>
* Updates Data Model to store toolState
* Executes Software PWM to control multiple motors handling tools
```
Current Tools  |Valid Param| Description|
| --- | --- | --- |
|1. Brush Servo | <toolId,status>                   | status: 1 - Lock, 0 -  Unlock
|2. Brush DC      | <toolId,status,direction,speed>   | status: 1 - ON, 0 - OFF,<br> direction: 1/0,<br> speed: 0 - 100
|3. Water Pump DC | <toolId,status,direction,speed>   | status: 1 - ON, 0 - OFF,<br> direction: 1/0,<br> speed: 0 - 100
|4. Soap Pump DC  | <toolId,status,direction,speed>   | status: 1 - ON, 0 - OFF,<br> direction - 1/0,<br> speed: 0 - 100




### Abluo Wheels Driver
```
Abluo Wheels Driver: Driver to command 4 wheel meccanum wheel robot (without encoder feedback)
@file abluo_wheels_driver.ino
@author Rishab Patwari (patwariri@gmail.com)
@references: https://www.baldengineer.com/software-pwm-with-millis.html
@version: 1.0 (1 June 2022)

  * Takes in Serial Input of the form <commandId,motionDirection,speed,duration(optional)>
  * Updates Data Model to store MotorState
  * Executes Software PWM to control multiple motors handling Motors
```

|   Command |   Valid Params    |   Description |
|   ---   | --- |   --- |
1       | <commandId,motionDirection, speed, duration > | Duration Based Movement in set Direction
2       | <commandId,motionDirection, speed >           | Continuous Movement in set Direction
3       | <commandId >                                  | Stop Movement


```
  Direction Mapping
  -------------------
  0 : FORWARD
  1 : BACKWARD
  2 : LEFT
  3 : RIGHT
  4 : CLOCKWISE
  5 : ANTI-CLOCKWISE
  ```