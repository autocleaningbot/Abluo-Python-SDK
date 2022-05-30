/***
 * @file abluo_Motors.ino
 * @brief Abluo Motor Control Driver for Arduino Mega
 *
 * Takes in Serial Input of the form <MotorId,status,direction,speed>
 * Updates Data Model to store MotorState
 * Executes Software PWM to control multiple motors handling Motors
 *
 *  Current Motors         |           Valid Params            |                   Description                         |
 * --------------------------------------------------------------------------------------------------------------
 * 1. M0  - Brush Servo   | <MotorId,status>                   | status: 1 - Lock, 0: Unlock
 * 2. M1A - Brush DC      | <MotorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 3. M1B - Water Pump DC | <MotorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 4. M2A - Wheel 1       | <MotorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 *
 * @author Rishab Patwari (patwariri@gmail.com)
 * @references: https://www.baldengineer.com/software-pwm-with-millis.html
 */
#include <Servo.h>
#include "src/SERIAL_PROCESSING.h"
#include "src/PWM_PROCESSING.h"

#define BAUD_RATE 115200
// DC Controller 1 - Motor 1
#define M1A_EN 39
#define M1_IN1 35
#define M1_IN2 37
// DC Controller 1 - Motor 2
#define M1B_EN 38
#define M1_IN3 36
#define M1_IN4 34

// DC Controller 2 - Motor 1
#define M2A_EN 50
#define M2_IN1 48
#define M2_IN2 46
// DC Controller 2 - Motor 2
#define M2B_EN 51
#define M2_IN3 47
#define M2_IN4 49

// Servo Controller Pin
#define SPIN 2


typedef void (*functiontype) ();
Servo servo;
unsigned long currentMicros = micros();

// Controller M1: Tool PWM Controller
// Frequency:(150 us x 100 Count = 15 ms Period / 66 Hz)
// const byte ToolPinsCount = 2;
// const byte ToolPwmPins[ToolPinsCount] = {M1A_EN, M1B_EN};
// PWM_PROCESSING pwmToolsController;

// Controller M2: Motor PWM Controller
// Frequency: (1us x 58 Count = 58 us Period / 17.2 kHz)
const byte motorMicroInterval = 1;
const byte motorPwmMax = 50;
const byte motorPinsCount = 4;
const byte motorPwmPins[motorPinsCount] = {M1A_EN, M2A_EN, M1B_EN, M2B_EN};
PWM_PROCESSING pwmMotorsController;

// Motor Data Structures
struct Motor
{
    int MotorId;
    int status;
    int direction;
    int speed;
};
const byte MotorCount = 5;
Motor myMotors[MotorCount];

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {M1_IN1, M1_IN2, M1_IN3, M1_IN4, M2_IN1, M2_IN2, M2_IN3, M2_IN4};

// Millis Timer
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long millisInterval = 250;

// Process Serial Input
void processNewData()
{
    if (newData == true)
    {
        parseInput();
        handleUpdate();
    }
    newData = false;
}

// Handle Millis Timer
void handleMillis()
{
    currentMillis = millis();
    if (currentMillis - previousMillis >= millisInterval)
    {
        previousMillis = currentMillis;
    }
}

// MOTOR CONTROLLERS DIRECTION & BRAKE
// MC 1A
void set_M1A_D1()
{
    digitalWrite(M1_IN1, HIGH);
    digitalWrite(M1_IN2, LOW);
}
void set_M1A_D2()
{
    digitalWrite(M1_IN1, LOW);
    digitalWrite(M1_IN2, HIGH);
}
void brake_M1A()
{
    digitalWrite(M1_IN1, LOW);
    digitalWrite(M1_IN2, LOW);
}
// MC 1B
void set_M1B_D1()
{
    digitalWrite(M1_IN3, HIGH);
    digitalWrite(M1_IN4, LOW);
}
void set_M1B_D2()
{
    digitalWrite(M1_IN3, LOW);
    digitalWrite(M1_IN4, HIGH);
}
void brake_M1B()
{
    digitalWrite(M1_IN3, LOW);
    digitalWrite(M1_IN4, LOW);
}

// MC 2A
void set_M2A_D1()
{
    digitalWrite(M2_IN1, HIGH);
    digitalWrite(M2_IN2, LOW);
}
void set_M2A_D2()
{
    digitalWrite(M2_IN1, LOW);
    digitalWrite(M2_IN2, HIGH);
}
void brake_M2A()
{
    digitalWrite(M2_IN1, LOW);
    digitalWrite(M2_IN2, LOW);
}
// MC 2B
void set_M2B_D1()
{
    digitalWrite(M2_IN3, HIGH);
    digitalWrite(M2_IN4, LOW);
}
void set_M2B_D2()
{
    digitalWrite(M2_IN3, LOW);
    digitalWrite(M2_IN4, HIGH);
}
void brake_M2B()
{
    digitalWrite(M2_IN3, LOW);
    digitalWrite(M2_IN4, LOW);
}

// Servo Motor
void onServo()
{
    servo.attach(SPIN);
    servo.write(135);
}
void offServo()
{
    servo.attach(SPIN);
    servo.write(179);
}

void setupMotors()
{
    for (int index = 1; index <= MotorCount; index++)
    {
        myMotors->MotorId = index;
        myMotors->status = 0;
        myMotors->direction = 0;
        myMotors->speed = 0;
    }
}

void handleDcMotor(int payload[], functiontype setDir_1, functiontype setDir_2, functiontype brake)
{
    int MotorIndex = payload[0] - 1;
    int pwmEnPinValue = MotorIndex - 1;
    int status = payload[1];
    int direction = payload[2];
    int speed = payload[3];

    myMotors[MotorIndex].status = status;
    myMotors[MotorIndex].speed = speed;
    myMotors[MotorIndex].direction = direction;

    if (myMotors[MotorIndex].status)
    {
        
        myMotors[MotorIndex].direction == 1 ? setDir_1(): setDir_2();
        pwmMotorsController.updatePinPwmValue(pwmEnPinValue, speed);
    }
    else
    {
        brake();
    }

}

void handleServo(int status, functiontype onServo, functiontype offServo)
{
    status == 1 ? onServo() : offServo();
    myMotors[0].status = status;
}

void handleUpdate()
{
    int MotorId = payload[0];
    switch (MotorId)
    {
    case 1: // Brush Servo
    {
        // handleServo();
        break;
    }
    case 2: // M1_A - Front Left
    {
        handleDcMotor(payload, set_M1A_D1, set_M1A_D2, brake_M1A);
        break;
    }
    case 3: // M1_B - Front Right
    {
        handleDcMotor(payload, set_M1B_D1, set_M1B_D2, brake_M1B);
        break;
    }
    case 4: // M2_A - Back Right
    {
        handleDcMotor(payload, set_M2A_D1, set_M2A_D2, brake_M2A);
        break;
    }
    case 5: // M2_B - Back Left
    {
        handleDcMotor(payload, set_M2B_D1, set_M2B_D2, brake_M2B);
        break;
    }
    default:
    {
        Serial.println("[ERROR] Invalid Motor ID - " + payload[0]);
        break;
    }
    }
    Serial.println("[DONE]");
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(BAUD_RATE);
    Serial.println("<Arduino is ready>");
    delay(1000);
    for (int index = 0; index < digitalPinCount; index++)
    {
        pinMode(digitalPins[index], OUTPUT);
    }
    pwmMotorsController.init(motorMicroInterval,motorPwmMax,motorPwmPins); //Old Value is 1,150
    // pwmToolsController.init(1,150, toolPwmPins);
}

void loop()
{
    currentMicros = micros();
    pwmMotorsController.handlePWM();
    // pwmToolsController.handlePWM();
    recvWithEndMarker();
    processNewData();
}