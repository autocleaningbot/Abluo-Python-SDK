#!/usr/bin/env python3
import inputs
import time
from importlib import reload
from abluo_sdk.abluo_api_i2c import abluoWheelsApi, abluoToolsApi, abluoEncodersApi

class AbluoController:
    def __init__(self, i2cBus, toolsAddr, wheelsAddr, encodersAddr, **kwargs):
        self.i2cBus = i2cBus
        self.toolsAddr = toolsAddr
        self.wheelsAddr = wheelsAddr
        self.encodersAddr = encodersAddr

    def linear(self, state):
        if state == -1:
            print ("Moving Forward")
            #code to move forward
            self.abluoWheels.MOTION_STATE.dir = 0
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

        if state == 1:
            print ("Moving Backward")
            #code to move backward
            self.abluoWheels.MOTION_STATE.dir = 1
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

    def sideway(self, state):
        if state == -1:
            print("Moving Left")
            #code to move left
            self.abluoWheels.MOTION_STATE.dir = 2
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

        if state == 1:
            print("Moving Right")
            #code to move right
            self.abluoWheels.MOTION_STATE.dir = 3
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

    def anti_clockwise(self, state):
        if state == 1:
            print("Turning anti-clockwise")
            #code to turn anti-clockwise
            self.abluoWheels.MOTION_STATE.dir = 9
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

    def clockwise(self, state):
        if state == 1:
            print("Turning Clockwise")
            #code to turn clockwise
            self.abluoWheels.MOTION_STATE.dir = 8
            self.abluoWheels.MOTION_STATE.speed = 15
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir
                                                    ,self.abluoWheels.MOTION_STATE.speed)

    def stop(self, state):
        if state == 1:
            print("Stop")
            #code to stop
            self.abluoWheels.stopAllWheels()

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
        if state == 1 and self.abluoWheels.moving == True:
            self.abluoWheels.MOTION_STATE.speed += 2
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir,self.abluoWheels.MOTION_STATE.speed)

    def decrease_speed(self, state):
        if state == 1 and self.abluoWheels.moving == True:
            self.abluoWheels.MOTION_STATE.speed -=2
            self.abluoWheels.sendContinuousMovementCommand(self.abluoWheels.MOTION_STATE.dir,self.abluoWheels.MOTION_STATE.speed)

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
            'BTN_NORTH' : get_speed,
    }

    def event_loop(self, events):
        '''
        This function is called in a loop, and will get the events from the
        controller and send them to the functions we specify in the `event_lut`
        dictionary
        '''
        for event in events: 
            #print('\t', event.ev_type, event.code, event.state)
            call = self.event_lut.get(event.code)
            if callable(call):
                call(self, event.state)

    def main(self):
        try:
            self.abluoWheels = abluoWheelsApi(self.i2cBus, self.wheelsAddr)
            self.abluoTools = abluoToolsApi(self.i2cBus, self.toolsAddr)
            self.abluoEncoders = abluoEncodersApi(self.i2cBus, self.encodersAddr)
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
