#ifndef PWM_PROCESSING_H
#define PWM_PROCESSING_H
#define ON true
#define OFF false
#include <Arduino.h>

struct pwmPin
{
    int pin;          // Pin Number
    int pwmValue;     // Pwm Value (duty cycle)
    bool pinState;    // Pin Output State
    int pwmTickCount; // PWM Counter Value
};

class PWM_PROCESSING
{
public:
    PWM_PROCESSING(){};
    ~PWM_PROCESSING(){};
    void init(int microInterval, int pwmMax, const byte *pwmPins, const byte pwmPinsCount);
    void handlePWM();
    void updatePinPwmValue(int pinIndex, int newPwmValue);
    void updatePinPwmDutyCycle(int pinIndex, int dutyCycle);
    void setupPWMpins(const byte *pwmPins);
private:
    unsigned long currentMicros;
    unsigned long previousMicros;
    unsigned long microInterval;
    pwmPin* myPWMpinsArray;
    int pwmPinCount;
    int pwmMax;
};
#endif