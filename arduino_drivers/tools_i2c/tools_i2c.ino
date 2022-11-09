/***
 * @file tools_i2c.ino
 * @brief Abluo Tool Control Driver for Arduino Mega
 *
 * Takes in Serial Input of the form <toolId,status,direction,speed>
 * Updates Data Model to store toolState
 * Executes Software PWM to control multiple motors handling tools
 *
 *  Current Tools         |           Valid Params            |                   Description                         |
 * --------------------------------------------------------------------------------------------------------------
 * 1. M0  - Brush Servo   | <toolId,status>                   | status: 1 - Lock, 0: Unlock
 * 2. M1A - Brush DC      | <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 3. M1B - Water Pump DC | <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 4. M2A - Wheel 1       | <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 *
 * @author Rishab Patwari (patwariri@gmail.com) & Wang Huachen (huachenw24@gmail.com)
 * @references: https://www.baldengineer.com/software-pwm-with-millis.html
 */
#define DEFINE_VARIABLES
#include <Servo.h>
#include <Wire.h>
#include "src/I2C_PROCESSING.h"
#include "src/PWM_PROCESSING.h"

// Motor Controller 1 - Tool Controller 1: DC Motor + Water Pump
#define M1A_EN 9
#define M1_IN1 2
#define M1_IN2 4
#define M1_IN3 7
#define M1_IN4 8
#define M1B_EN 10

// Servo Controller Pin
#define SPIN 5

// Motor Controller 2 - Wheel Controller 1
#define M2A_EN 11
#define M2_IN1 13
#define M2_IN2 12
#define M2_IN3 0
#define M2_IN4 6
#define M2B_EN 1

#define ESTOP_INT 3

// Helpers
bool settingUp = true;
bool emergency = false;
unsigned long last_interrupt_time = millis();
Servo servo;
unsigned long currentMicros = micros();

// Controller M1: Tool PWM Controller
// Frequency:(150 us x 100 Count = 15 ms Period / 66 Hz)
const byte toolPinsCount = 2;
const byte toolPwmPins[toolPinsCount] = {M1A_EN, M1B_EN};
PWM_PROCESSING pwmToolsController;

// Controller M2: Wheel PWM Controller
// Frequency: (1us x 58 Count = 58 us Period / 17.2 kHz)
const byte wheelMicroInterval = 1;
const byte wheelPwmMax = 58;
const byte wheelPinsCount = 2;
const byte wheelPwmPins[wheelPinsCount] = {M2A_EN, M2B_EN};
PWM_PROCESSING pwmWheelsController;

// Tool Data Structures
struct tool
{
    int toolId;
    int status;
    int direction;
    int speed;
};
const byte toolCount = 5;
tool myTools[toolCount];

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {M1_IN1, M1_IN2, M1_IN3, M1_IN4, M2_IN1, M2_IN2, M2_IN3, M2_IN4};

// Millis Timer
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long millisInterval = 1;
unsigned long servoAttachWait = 0;

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
        handleServoAttachWait();
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
    servo.write(115);
    servoAttachWait = 1000;
    // delay(1000)
}
void offServo()
{
    servo.attach(SPIN);
    servo.write(185);
    servoAttachWait = 1000;
    // delay(1000)
}

void setupTools()
{
    for (int index = 1; index <= toolCount; index++)
    {
        myTools->toolId = index;
        myTools->status = 0;
        myTools->direction = 0;
        myTools->speed = 0;
    }
}

void handleUpdate()
{
    int toolId = payload[0];
    int status = payload[1];
    int direction = payload[2];
    int speed = payload[3];
    switch (toolId)
    {
    case 1: // Brush Servo
    {
        status == 1 ? onServo() : offServo();
        myTools[0].status = status;
        break;
    }
    case 2: // M1_A - Brush Dc
    {
        myTools[1].status = status;
        myTools[1].speed = speed;
        myTools[1].direction = direction;
        // Update M1A_EN Pin based on status and speed

        if (myTools[1].status == 1)
        {
            myTools[1].direction == 1 ? set_M1A_D1() : set_M1A_D2();
            pwmToolsController.updatePinPwmValue(0, myTools[1].speed);
        }
        else
        {
            brake_M1A();
        }
        break;
    }
    case 3: // M1_B - Water Pump
    {
        myTools[2].status = status;
        myTools[2].speed = speed;
        myTools[2].direction = direction;
        // Update M1B_EN Pin based on status and speed
        if (myTools[2].status == 1)
        {
            myTools[2].direction == 1 ? set_M1B_D1() : set_M1B_D2();
            pwmToolsController.updatePinPwmValue(1, myTools[2].speed);
        }
        else
        {
            brake_M1B();
        }
        break;
    }
    case 4: // M2_A - Wheel 1 (Front-Left)
    {
        myTools[3].status = status;
        myTools[3].speed = speed;
        myTools[3].direction = direction;
        // Update M2A_EN Pin based on status and speed
        if (myTools[3].status)
        {
            myTools[3].direction == 1 ? set_M2A_D1() : set_M2A_D2();
            pwmToolsController.updatePinPwmValue(2, myTools[3].speed);
        }
        else
        {
            brake_M2A();
        }
        break;
    }
    case 5: // M2_B - Wheel 2 (Front-Right)
    {
        myTools[4].status = status;
        myTools[4].speed = speed;
        myTools[4].direction = direction;
        // Update M2A_EN Pin based on status and speed
        if (myTools[4].status)
        {
            myTools[4].direction == 1 ? set_M2B_D1() : set_M2B_D2();
            pwmToolsController.updatePinPwmValue(3, myTools[4].speed);
        }
        else
        {
            brake_M2B();
        }
        break;
    }
    default:
    {
        // Serial.println("[ERROR] Invalid Tool ID - " + payload[0]);
        break;
    }
    }
    // Serial.println("[DONE]");
}

void emergencyStop()
{
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 50)
    {
        last_interrupt_time = interrupt_time;
        if (digitalRead(ESTOP_INT) == LOW)
        {
            brake_M1A();
            brake_M1B();
            brake_M2A();
            brake_M2B();
            emergency = true;
        }
    }
}

void setup()
{
    // put your setup code here, to run once:
    Wire.begin(0x60);
    Wire.setClock(400000);
    Wire.onReceive(receiveEvent);
    for (int index = 0; index < digitalPinCount; index++)
    {
        pinMode(digitalPins[index], OUTPUT);
    }
    pinMode(ESTOP_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ESTOP_INT), emergencyStop, FALLING);
    pwmToolsController.init(150, 100, toolPwmPins);
    pwmWheelsController.init(1, 58, wheelPwmPins);
    settingUp = false;
}

void handleServoAttachWait()
{
    if (servoAttachWait > 0)
    {
        servoAttachWait--;
        if (servoAttachWait = 0)
        {
            servo.detach();
        }
    }
}

void loop()
{
    if (digitalRead(ESTOP_INT) == LOW)
    {
      brake_M1A();
      brake_M1B();
      brake_M2A();
      brake_M2B();
      emergency = true;
    }
    else
    {
      emergency = false;
    }
    if (!settingUp)
    {
      if (!emergency)
      {
        currentMicros = micros();
        pwmToolsController.handlePWM();
        pwmWheelsController.handlePWM();
        processNewData();
        handleMillis();
      }
      else
      {
        parseInput();
      }
    }
}
