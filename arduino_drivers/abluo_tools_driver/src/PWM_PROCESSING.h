#ifndef PWM_PROCESSING_H
#define PWM_PROCESSING_H
#define ON true
#define OFF false
#define MOTOR_CONTROLLER_PWM_PIN_COUNT 2
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
    int pwmPinCount;
    byte pwmMax;

public:
    PWM_PROCESSING(){};
    ~PWM_PROCESSING(){};
    void init(int microInterval, int pwmMax, const byte* pwmPins);
    void handlePWM();
    void updatePinPwmValue(int pinIndex, int newPwmValue);
    void setupPWMpins(const byte* pwmPins);

private:
    unsigned long previousMicros;
    unsigned long microInterval;
    pwmPin myPWMpinsArray[MOTOR_CONTROLLER_PWM_PIN_COUNT];
};
#endif