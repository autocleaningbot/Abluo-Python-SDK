#!/usr/bin/env python3
import inputs
import time
from importlib import reload
from enum import Enum
from abluo_sdk import  abluoWheels, abluoEncoders

WHEEL_INITIAL_PWM = 15
WHEEL_MAX_PWM = 25
WHEEL_MIN_PWM = 13

class AbluoDir(Enum):
    STOP = 0
    FORWARD = 1
    BACKWARD = 2
    LEFT = 3
    RIGHT = 4
    ANTICLOCKWISE = 5
    CLOCKWISE = 6

class AbluoController:
    def __init__(self, i2cBus, wheelsAddr, encodersAddr, **kwargs):
        self.i2cBus = i2cBus
        self.wheelsAddr = wheelsAddr
        self.encodersAddr = encodersAddr
        self.wheelPWM = WHEEL_INITIAL_PWM
        self.currentDir = AbluoDir.STOP

    def linear(self, state):
        if state == -1:
            print ("Moving Forward")
            #code to move forward
            self.abluoWheels.sendCommand([self.wheelPWM, self.wheelPWM, self.wheelPWM, self.wheelPWM])
            self.currentDir = AbluoDir.FORWARD

        if state == 1:
            print ("Moving Backward")
            #code to move backward
            self.abluoWheels.sendCommand([-self.wheelPWM, -self.wheelPWM, -self.wheelPWM, -self.wheelPWM])
            self.currentDir = AbluoDir.BACKWARD

    def sideway(self, state):
        if state == -1:
            print("Moving Left")
            #code to move left
            self.abluoWheels.sendCommand([-self.wheelPWM, self.wheelPWM, self.wheelPWM, -self.wheelPWM])
            self.currentDir = AbluoDir.LEFT

        if state == 1:
            print("Moving Right")
            #code to move right
            self.abluoWheels.sendCommand([self.wheelPWM, -self.wheelPWM, -self.wheelPWM, self.wheelPWM])
            self.currentDir = AbluoDir.RIGHT

    def anti_clockwise(self, state):
        if state == 1:
            print("Turning anti-clockwise")
            #code to turn anti-clockwise
            self.abluoWheels.sendCommand([-self.wheelPWM, self.wheelPWM, -self.wheelPWM, self.wheelPWM])
            self.currentDir = AbluoDir.ANTICLOCKWISE

    def clockwise(self, state):
        if state == 1:
            print("Turning Clockwise")
            #code to turn clockwise
            self.abluoWheels.sendCommand([self.wheelPWM, -self.wheelPWM, self.wheelPWM, -self.wheelPWM])
            self.currentDir = AbluoDir.CLOCKWISE

    def stop(self, state):
        if state == 1:
            print("Stop")
            #code to stop
            self.abluoWheels.stop()
            self.currentDir = AbluoDir.STOP

    def pick_brush(self, state):
        if state == 1:
            print("Pick Brush")
            #code

    def drop_brush(self, state):
        if state == 1:
            print("Drop Brush")
            #code

    def clean_toilet(self, state):
        if state == 1:
            print("Clean Toilet")
            #code

    def water_toilet(self, state):
        if state == 1:
            print("water toilet")
            #code

    def water_urinal(self, state):
        if state == 1:
            print("water urinal")
            #code

    def increase_speed(self, state):
        # Change speed of future commands
        if state == 1:
            self.wheelPWM += 2
            if self.wheelPWM < WHEEL_MIN_PWM:
                self.wheelPWM = WHEEL_MIN_PWM
            if self.wheelPWM > WHEEL_MAX_PWM:
                self.wheelPWM = WHEEL_MAX_PWM
            print("Current Speed: %d" % self.wheelPWM)
            self.continue_current_dir()

    def decrease_speed(self, state):
        # Change speed of future commands
        if state == 1:
            self.wheelPWM -= 2
            if self.wheelPWM < WHEEL_MIN_PWM:
                self.wheelPWM = WHEEL_MIN_PWM
            if self.wheelPWM > WHEEL_MAX_PWM:
                self.wheelPWM = WHEEL_MAX_PWM
            print("Current Speed: %d" % self.wheelPWM)
            self.continue_current_dir()

    def continue_current_dir(self):
        if self.currentDir == AbluoDir.STOP:
            self.abluoWheels.stop()
        elif self.currentDir == AbluoDir.FORWARD:
            self.abluoWheels.sendCommand([self.wheelPWM, self.wheelPWM, self.wheelPWM, self.wheelPWM])
        elif self.currentDir == AbluoDir.BACKWARD:
            self.abluoWheels.sendCommand([-self.wheelPWM, -self.wheelPWM, -self.wheelPWM, -self.wheelPWM])
        elif self.currentDir == AbluoDir.LEFT:
            self.abluoWheels.sendCommand([-self.wheelPWM, self.wheelPWM, self.wheelPWM, -self.wheelPWM])
        elif self.currentDir == AbluoDir.RIGHT:
            self.abluoWheels.sendCommand([self.wheelPWM, -self.wheelPWM, -self.wheelPWM, self.wheelPWM])
        elif self.currentDir == AbluoDir.ANTICLOCKWISE:
            self.abluoWheels.sendCommand([-self.wheelPWM, self.wheelPWM, -self.wheelPWM, self.wheelPWM])
        elif self.currentDir == AbluoDir.CLOCKWISE:
            self.abluoWheels.sendCommand([self.wheelPWM, -self.wheelPWM, self.wheelPWM, -self.wheelPWM])


    def get_speed(self, state):
        if state == 1:
            fl, fr, bl, br = self.abluoEncoders.readEncoders()
            print("Getting speeds...")
            print("Front Left: ", fl, " rad/s")
            print("Front Right: ", fr, "rad/s")
            print("Back Left: ", bl, "rad/s")
            print("Back Right: ", br, "rad/s")

    '''
    Set which actions happen when buttons and joysticks change:
    DirectInput
    '''
    #event_lut = {
    #    'ABS_HAT0Y' : linear,
    #    'ABS_HAT0X' : sideway,
    #    'BTN_WEST' : anti_clockwise,
    #    'BTN_Z' : clockwise,
    #    'BTN_C' : stop,
    #    'BTN_SOUTH' : pick_brush,
    #    'BTN_NORTH' : drop_brush,
    #    'BTN_EAST' : clean_toilet,
    #    'BTN_TL' : water_toilet,
    #    'BTN_TR' : water_urinal,
    #    'BTN_TR2': increase_speed,
    #    'BTN_TL2': decrease_speed,
    #}

    '''
    XInput
    '''
    event_lut = {
            'ABS_HAT0Y' : linear,
            'ABS_HAT0X' : sideway,
            'BTN_TL' : anti_clockwise,
            'BTN_TR' : clockwise,
            'BTN_EAST' : stop,
            'BTN_WEST' : increase_speed,
            'BTN_SOUTH' : decrease_speed,
            #'BTN_NORTH' : get_speed,
    }

    def event_loop(self, events):
        '''
        This function is called in a loop, and will get the events from the
        controller and send them to the functions we specify in the `event_lut`
        dictionary
        '''
        for event in events: 
            call = self.event_lut.get(event.code)
            if callable(call):
                call(self, event.state)

    def main(self):
        try:
            self.abluoWheels = abluoWheels(self.i2cBus, self.wheelsAddr)
            self.abluoEncoders = abluoEncoders(self.i2cBus, self.encodersAddr)
            setupTries = 5
            while setupTries > 0:
                if setupTries < 5:
                    reload(inputs)
                pads = inputs.devices.gamepads
                #check if gamepad is connected
                if len(pads) == 0:
                    print("Gamepad not found, trying %i more times." % setupTries)
                elif "Logitech Gamepad F710" in pads[0].name:
                    break
                else:
                    print("Wrong input mode/controller detected, trying %i more times." % setupTries)
                    print(pads[0].name)
                time.sleep(3)
                setupTries-=1
                if setupTries == 0:
                    print("Exceeded try count, please check controller and restart program.")
                    exit()
            print("Controller ready")
            try: 
                while True:
                    self.event_loop(inputs.get_gamepad())
            except inputs.UnpluggedError:
                print("No controller connected.")
            except OSError:
                print("Controller disconnected. Exiting.")
        except KeyboardInterrupt:
            pass
