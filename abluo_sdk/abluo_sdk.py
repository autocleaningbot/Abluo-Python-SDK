#!/usr/bin/env python3
# Software License Agreement (BSD License)
#
# Copyright (c) 2022, HiveBotics, Inc.
# All rights reserved.
#
# Author: Tuan Dung <dung.nguyen.t@hivebotics.tech>

import time
import serial
import smbus2 as smbus

class abluoTools:
    def __init__(self, port='/dev/ttyACM0',baudRate=9600, **kwargs):
        """
        Sends commands to tool controller

        """
        self._serconn = serial.Serial(port, baudRate, timeout=1)
        time.sleep(2.5)

    def __del__(self):
        self._serconn.close()
        print("[INFO] Closed Serial Connection With Microcontroller")

    def sendCommand(self, tool, value):
        payload = "{:03d},{:03d}\n".format(tool, value)
        payload_byte = []
        for c in payload:
            payload_byte.append(ord(c))
        self._serconn.write(payload.encode())
        time.sleep(0.5)

class abluoWheels:
    def __init__(self, i2cBus, i2cAddress, **kwargs):
        """
        Sends commands to wheel controller

        :param i2cBus: bus of I2C on master
        :param i2cAddress: i2c address for connected microcontroller
        """
        self._i2cBus = i2cBus
        self._i2cAddress = i2cAddress
        self.wire = smbus.SMBus(i2cBus)
        time.sleep(1)

    def sendCommand(self, vel):
        vel = [int(i) for i in vel]
        payload = "{:03d},{:03d},{:03d},{:03d}\n".format(vel[0], vel[1], vel[2], vel[3])
        payload_byte = []
        for c in payload:
            payload_byte.append(ord(c))
        self.wire.write_i2c_block_data(self._i2cAddress, 0x00, payload_byte)

    def stop(self):
        self.sendCommand([0, 0, 0, 0])

class abluoEncoders:
    def __init__(self, i2cBus, i2cAddress, **kwargs):
        """
        Get encoder data

        :param i2cBus: bus of I2C on master
        :param i2cAddress: i2c address for connected microcontroller
        """
        self._i2cBus = i2cBus
        self._i2cAddress = i2cAddress
        self.wire = smbus.SMBus(i2cBus)
        time.sleep(1)

    def readEncoders(self):
        read = self.wire.read_i2c_block_data(self._i2cAddress, 0x00, 31)
        spdbytes = list(read)
        spdstring = bytearray(spdbytes).decode("ascii")
        velocities = spdstring.split(',')
        try:
            velocities = [float(vel) for vel in velocities]
            return velocities
        except:
            print(velocities)
            return "NO DATA"

if __name__ == "__main__":
    # For testing purposes
    # wheels = abluoWheels(1, 112)
    # tools = abluoTools(1,96)
    tools = abluoTools(port="/dev/ttyACM0")
    #encoders = abluoEncoders(1, 112)
    looping = True
    i = 0
    tools.sendCommand(0,100)
    tools.sendCommand(1,180)
    tools.sendCommand(1,90)
    tools.sendCommand(0,0)
    # tools.sendCommand(2,1)
    # time.sleep(25)
    # tools.sendCommand(2,0)
    # while looping:
    #     # print(i)
    #     wheels.sendCommand([-15, 15, -15, 15])
    #     #print(encoders.readEncoders())
    #     time.sleep(0.01)
    #     i+=1
    #     if i == 100:
    #         looping = False
    # wheels.stop()
