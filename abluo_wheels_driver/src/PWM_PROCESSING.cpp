#include "PWM_PROCESSING.h"

/**
 * @brief Construct a new pwm processing::pwm processing object
 *
 * @param microInterval
 * @param pwmMax
 * @param pwmPins
 */
void PWM_PROCESSING::init(int microInterval, int pwmMax, const byte *pwmPins, const byte pwmPinsCount)
{
    this->previousMicros = 0;
    this->microInterval = microInterval;
    this->pwmMax = pwmMax;
    this->pwmPinCount = pwmPinsCount;
    this->setupPWMpins(pwmPins);
    this->currentMicros = micros();
}

/**
 * Execute Software PWM Loop
 * ...
 *
 * Increments pwmTickCount for each PWM Pin
 * If ON: If pwmTickCount >= pwmValue for pin, sets pinState to OFF
 *  (ie. if pwmValue = 125, btw 125 and 255 will OFF)
 * If OFF: If pwmTickCount >= pwmMax for pin, sets pinState to ON
 *  (ie. exceed 255)
 */
void PWM_PROCESSING::handlePWM()
{
    currentMicros = micros();
    if (currentMicros - previousMicros >= microInterval)
    {
        for (int index = 0; index < pwmPinCount; index++)
        {
            myPWMpinsArray[index].pwmTickCount++;
            if (myPWMpinsArray[index].pinState == ON)
            {
                if (myPWMpinsArray[index].pwmTickCount >= myPWMpinsArray[index].pwmValue)
                {
                    myPWMpinsArray[index].pinState = OFF;
                }
            }
            else
            {
                if (myPWMpinsArray[index].pwmTickCount >= pwmMax)
                {
                    myPWMpinsArray[index].pinState = ON;
                    myPWMpinsArray[index].pwmTickCount = 0;
                }
            }
            // digitalWrite(myPWMpinsArray[index].pin, myPWMpinsArray[index].pinState);
        }
        previousMicros = currentMicros;
    }
}

void PWM_PROCESSING::updatePinPwmValue(int pinIndex, int newPwmValue)
{
    myPWMpinsArray[pinIndex].pwmValue = newPwmValue;
}
void PWM_PROCESSING::updatePinPwmDutyCycle(int pinIndex, int dutyCycle)
{
    myPWMpinsArray[pinIndex].pwmValue = dutyCycle / pwmMax * 100;
}

void PWM_PROCESSING::setupPWMpins(const byte *pwmPins)
{
    for (int index = 0; index < pwmPinCount; index++)
    {
        myPWMpinsArray[index] = (pwmPin){.pin = pwmPins[index], .pwmValue = 255, .pinState = ON, .pwmTickCount = 0};
        pinMode(pwmPins[index], OUTPUT);
        // sprintf(buffer, "PWM_PIN: %d [pwmVal: %d, pinState: %d, pwmTickCount: %d]", myPWMpinsArray[index].pin, myPWMpinsArray[index].pwmValue, myPWMpinsArray[index].pinState, myPWMpinsArray[index].pwmTickCount);
        // Serial.println(buffer);
    }
}