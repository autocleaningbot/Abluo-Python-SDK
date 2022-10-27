#!/usr/bin/env python3
import inputs
import time
from importlib import reload
from abluo_sdk.abluo_api_i2c import abluoWheelsApi, abluoToolsApi, abluoEncodersApi

def linear(state):
    if state == -1:
        print ("Moving Forward")
        #code to move forward
        abluoWheels.MOTION_STATE.dir = 0
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

    if state == 1:
        print ("Moving Backward")
        #code to move backward
        abluoWheels.MOTION_STATE.dir = 1
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

def sideway(state):
    if state == -1:
        print("Moving Left")
        #code to move left
        abluoWheels.MOTION_STATE.dir = 2
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

    if state == 1: 
        print("Moving Right")
        #code to move right
        abluoWheels.MOTION_STATE.dir = 3
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

def anti_clockwise(state):
    if state == 1:
        print("Turning anti-clockwise")
        #code to turn anti-clockwise
        abluoWheels.MOTION_STATE.dir = 9
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

def clockwise(state):
    if state == 1:
        print("Turning Clockwise")
        #code to turn clockwise
        abluoWheels.MOTION_STATE.dir = 8
        abluoWheels.MOTION_STATE.speed = 15
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir
                                                ,abluoWheels.MOTION_STATE.speed)

def stop(state):
    if state == 1:
        print("Stop")
        #code to stop
        abluoWheels.stopAllWheels()

def pick_brush(state):
    if state == 1:
        print("Pick Brush")
        #code

def drop_brush(state):
    if state == 1:
        print("Drop Brush")
        #code

def clean_toilet(state):
    if state == 1:
        print("Clean Toilet")
        #code

def water_toilet(state):
    if state == 1:
        print("water toilet")
        #code

def water_urinal(state):
    if state == 1:
        print("water urinal")
        #code

def increase_speed(state):
    if state == 1:
        abluoWheels.MOTION_STATE.speed += 2
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir,abluoWheels.MOTION_STATE.speed)

def decrease_speed(state):
    if state == 1:
        abluoWheels.MOTION_STATE.speed -=2
        abluoWheels.sendContinuousMovementCommand(abluoWheels.MOTION_STATE.dir,abluoWheels.MOTION_STATE.speed)

def get_speed(state):
    if state == 1:
        fl, fr, bl, br = abluoEncoders.readEncoders()
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

def event_loop(events):
    '''
    This function is called in a loop, and will get the events from the
    controller and send them to the functions we specify in the `event_lut`
    dictionary
    '''
    for event in events: 
        #print('\t', event.ev_type, event.code, event.state)
        call = event_lut.get(event.code)
        if callable(call):
            call(event.state)

if __name__ == '__main__':
    abluoWheels = abluoWheelsApi(1, 0x70)
    abluoEncoders = abluoEncodersApi(1, 0x65)
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
    #abluoWheels.sendWheelMotorStatus(0, 15, 3)
    #fl, fr, bl, br = abluoEncoders.readEncoders()
    #print(fl)
    #print(fr)
    #print(bl)
    #print(br)

    try: 
        while True:
            event_loop(inputs.get_gamepad())
    except OSError:
        print("Controller disconnected. Exiting.")
    except KeyboardInterrupt:
        print("Bye!")
