#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2022, HiveBotics, Inc.
# All rights reserved.
#
# Author: Rishab <rishab@hivebotics.tech> <patwariri@gmail.com>

import enum
from tokenize import String
import serial
import time


class abluoTool:
    def __init__(self, toolId, toolName, serialPort):
        self._toolId = toolId
        self._toolName = toolName
        self._status = 0
        self._direction = 0  # Possible Directions: < 0, 1>
        self._speed = 0  # Speed is Positive Integer between 0 and 100
        self._serialPort = serialPort

    def setStatus(self, newStatus):
        self._status = newStatus
        self.sendSerial()
        return self.toString()

    def setDirection(self, newDirection):
        self._direction = newDirection
        self.sendSerial()
        return self.toString()

    def setSpeed(self, newSpeed):
        self._speed = newSpeed
        self.sendSerial()
        return self.toString()

    def setAll(self, status, direction, speed):
        """Sets (status, direction, speed)"""
        self._status = status
        self._direction = direction
        self._speed = speed
        self.sendSerial()
        return self.toString()

    def sendSerial(self):
        payload = "{},{},{},{}\n".format(
            self._toolId, self._status, self._direction, self._speed)
        # payload = (b'sending string to Arduino')
        self._serialPort.write(payload.encode())
        time.sleep(0.5)

    def toString(self):
        toolInfo = """-----\n
        Id      : {}\n
        Name    : {}\n
        Status  : {}\n
        Speed   : {}\n
        
        """.format(self._toolId, self._toolName, self._status, self._speed)
        return toolInfo


class abluoToolsApi:
    def __init__(self, port='/dev/ttyACM0', baudRate=115200, **kwargs):
        """
        The python wrapper controller of Abluo

        :param port: port name of connected serial microcontroller
        :param baudRate: baudRate for connected serial microcontroller
        """
        self._serconn = serial.Serial(port, baudRate, timeout=1)
        self.tool1 = abluoTool(1, "Brush Servo", self._serconn)
        self.tool2 = abluoTool(2, "Brush Motor", self._serconn)
        self.tool3 = abluoTool(3, "Water Pump", self._serconn)
        self.tool4 = abluoTool(4, "Soap Pump", self._serconn)
        self._toolslist = [self.tool1, self.tool2, self.tool3, self.tool4]

    def setAllTools(self, *commandsArray):
        """Sets multiple tools at the same time. Requires (toolId, status, direction , speed)"""
        for commands in commandsArray:
            for tool in self._toolslist:
                if tool._toolId == commands[0]:
                    tool.setAll(commands[1], commands[2], commands[3])

    def stopAllTools(self):
        for tool in self._toolslist:
            tool.setStatus(0)

    def __del__(self):
        self._serconn.close()
        print("[INFO] Closed Serial Connection With Microcontroller")
    
    def testTools(self):
        # Test Brush Servo
        print("[TEST] Brush Servo - Started")
        print("[TEST] Brush Servo - TURN ON")
        self.tool1.setStatus(1)
        time.sleep(1.5)
        print("[TEST] Brush Servo - TURN OFF")
        self.tool1.setStatus(0)
        time.sleep(1.5)
        print("[TEST] Brush Servo - TURN ON")
        self.tool1.setStatus(1)
        time.sleep(1.5)
        print("[TEST] Brush Servo - TURN OFF")
        self.tool1.setStatus(0)
        time.sleep(1)
        print("[TEST] Brush Servo - Completed\n")

        # # Test Brush Motor
        print("[TEST] Brush DC - Started")
        print("[TEST] Brush DC - Direction 1, Max Speed")
        self.tool2.setAll(1, 0, 100)
        time.sleep(1)
        print("[TEST] Brush DC - Direction 2, Max Speed")
        self.tool2.setAll(1, 1, 100)
        time.sleep(1)
        print("[TEST] Brush DC - Speed Test [75%, 50%, 10%, STOP]")
        self.tool2.setAll(1, 1, 75)
        time.sleep(1)
        self.tool2.setAll(1, 1, 50)
        time.sleep(1)
        self.tool2.setAll(1, 1, 10)
        time.sleep(1)
        self.tool2.setStatus(0)
        print("[TEST] Brush DC - Completed\n")

        # # Test Water Pump 1
        print("[TEST] Water Pump 1 DC - Started")
        print("[TEST] Water Pump 1 DC - Direction 1, Max Speed")
        self.tool3.setAll(1, 0, 100)
        time.sleep(1)
        print("[TEST] Water Pump 1 DC - Direction 2, Max Speed")
        self.tool3.setAll(1, 1, 100)
        time.sleep(1)
        print("[TEST] Water Pump 1 DC - Speed Test [75%, 50%, STOP]")
        self.tool3.setAll(1, 1, 75)
        time.sleep(1)
        self.tool3.setAll(1, 1, 50)
        time.sleep(1)
        self.tool3.setStatus(0)
        print("[TEST] Water Pump 1 DC - Completed\n")

        # Test Water Pump & DC Motor PWM Working together
        print("[TEST] Mult PWM - Started")
        self.setAllTools([2, 1, 0, 90], [3, 1, 0, 90])
        time.sleep(1)
        self.stopAllTools()
        print("[TEST] Multi PWM - Completed\n")


class abluoWheelsApi:
    class WHEEL_INDEX():
        FRONT_LEFT = 0
        FRONT_RIGHT = 1
        BACK_LEFT = 2
        BACK_RIGHT = 3
    
    class MOTION_DIRECTION():
        FORWARD = 0
        BACKWARD = 1
        LEFT = 2
        RIGHT = 3
        FORWARD_RIGHT = 4
        FORWARD_LEFT = 5
        BACKWARD_RIGHT = 6
        BACKWARD_LEFT = 7
        CLOCKWISE = 8
        ANTI_CLOCKWISE = 9
    
    class COMMAND_IDS():
        MOVEMENT_WITH_DURATION = 1
        CONTINOUS_MOVEMENT = 2
        STOP_WHEELS = 3
        UPDATE_PARTICULAR_WHEEL = 4
    
    class WHEEL_DIRECTION():
        FORWARD = 0
        BACKWARD = 1

    class MOTION_STATE():
        speed = 15
        dir = 0



    def __init__(self, port='/dev/ttyACM0', baudRate=115200, **kwargs):
        """
        Sends commands to wheel controller

        :param port: port name of connected serial microcontroller
        :param baudRate: baudRate for connected serial microcontroller
        """
        self._serconn = serial.Serial(port, baudRate, timeout=1)

    def sendDurationMovementCommand(self, motionDirection, speed, duration):
        self.sendSerial(self.COMMAND_IDS.MOVEMENT_WITH_DURATION, motionDirection, speed, duration)

    def sendContinuousMovementCommand(self, motionDirection, speed):
        self.sendSerial(self.COMMAND_IDS.CONTINOUS_MOVEMENT, motionDirection, speed)

    def sendWheelMotorStatus(self, wheelDirection, speed, motorIndex, status):
        self.sendSerial(self.COMMAND_IDS.UPDATE_PARTICULAR_WHEEL, wheelDirection, speed, motorIndex, status)

    def stopAllWheels(self):
        self.sendSerial(self.COMMAND_IDS.STOP_WHEELS)

    def sendSerial(self, command, param1=0, param2=0, param3=0):
        payload = "{},{},{},{}\n".format(command, param1, param2, param3)
        # payload = (b'sending string to Arduino')
        self._serconn.write(payload.encode())

    def __del__(self):
        self._serconn.close()
        print("[INFO] Closed Serial Connection With Microcontroller")

    def testMovement(self):
        # Go Forward
        self.sendDurationMovementCommand(self.MOTION_DIRECTION.FORWARD, 15, 1500)
        # Go Right
        self.sendDurationMovementCommand(self.MOTION_DIRECTION.RIGHT, 20, 1500)
        # Go Backward
        self.sendDurationMovementCommand(self.MOTION_DIRECTION.BACKWARD,15,1500)
        # Go Left




if __name__ == "__main__":
    abluoTools = abluoToolsApi(port='/dev/ttyACM0')
    # abluoWheels = abluoWheelsApi(port='/dev/ttyACM1')
    abluoTools.testTools()