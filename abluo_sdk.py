#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2022, HiveBotics, Inc.
# All rights reserved.
#
# Author: Rishab <rishab@hivebotics.tech> <patwariri@gmail.com>

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
        # time.sleep(0.5)

    def toString(self):
        toolInfo = """-----\n
        Id      : {}\n
        Name    : {}\n
        Status  : {}\n
        Speed   : {}\n
        
        """.format(self._toolId, self._toolName, self._status, self._speed)
        return toolInfo


class abluoToolsApi:
    def __init__(self, port='/dev/ttyACM0', baudRate=2000000, **kwargs):
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

if __name__ == "__main__":
    abluoTools = abluoToolsApi()

    # Test Brush Servo
    print("[TEST] Brush Servo - Started")
    abluoTools.tool1.setStatus(0)
    time.sleep(1.5)
    abluoTools.tool1.setStatus(1)
    time.sleep(1.5)
    abluoTools.tool1.setStatus(0)
    time.sleep(1)
    print("[TEST] Brush Servo - Completed\n")

    # # Test Brush Motor
    print("[TEST] Brush DC - Started")
    print("[TEST] Brush DC - Direction 1, Max Speed")
    abluoTools.tool2.setAll(1, 0, 100)
    time.sleep(1)
    print("[TEST] Brush DC - Direction 2, Max Speed")
    abluoTools.tool2.setAll(1, 1, 100)
    time.sleep(1)
    print("[TEST] Brush DC - Speed Test [75%, 50%, 10%, STOP]")
    abluoTools.tool2.setAll(1, 1, 75)
    time.sleep(1)
    abluoTools.tool2.setAll(1, 1, 50)
    time.sleep(1)
    abluoTools.tool2.setAll(1, 1, 10)
    time.sleep(1)
    abluoTools.tool2.setStatus(0)
    print("[TEST] Brush DC - Completed\n")

    # # Test Water Pump 1
    print("[TEST] Water Pump 1 DC - Started")
    print("[TEST] Water Pump 1 DC - Direction 1, Max Speed")
    abluoTools.tool3.setAll(1, 0, 100)
    time.sleep(1)
    print("[TEST] Water Pump 1 DC - Direction 2, Max Speed")
    abluoTools.tool3.setAll(1, 1, 100)
    time.sleep(1)
    print("[TEST] Water Pump 1 DC - Speed Test [75%, 50%, STOP]")
    abluoTools.tool3.setAll(1, 1, 75)
    time.sleep(1)
    abluoTools.tool3.setAll(1, 1, 50)
    time.sleep(1)
    abluoTools.tool3.setStatus(0)
    print("[TEST] Water Pump 1 DC - Completed\n")

    # Test Water Pump & DC Motor PWM Working together
    print("[TEST] Mult PWM - Started")
    abluoTools.setAllTools([2, 1, 0, 90], [3, 1, 0, 90])
    time.sleep(1)
    abluoTools.stopAllTools()
    print("[TEST] Multi PWM - Completed\n")
