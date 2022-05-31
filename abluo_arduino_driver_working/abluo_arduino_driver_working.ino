/***
 * @file abluo_tools.ino
 * @brief Abluo Tool Control Driver for Arduino Mega
 *
 * Takes in Serial Input of the form <toolId,status,direction,speed>
 * Updates Data Model to store toolState
 * Executes Software PWM to control multiple motors handling tools
 *
 *  Current Tools   |           Valid Params            |                   Description                         |
 * --------------------------------------------------------------------------------------------------------------
 * 1. Brush Servo   : <toolId,status>                   | status: 1 - Lock, 0: Unlock
 * 2. Brush DC      : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 3. Water Pump DC : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 * 4. Soap Pump DC  : <toolId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
 *
 * @author Rishab Patwari (patwariri@gmail.com)
 * @references: https://www.baldengineer.com/software-pwm-with-millis.html
 */

#include <Servo.h>

// Motor Controller 1
#define ENA 9
#define ENB 38
#define IN1 10
#define IN2 11
#define IN3 36
#define IN4 34

// Motor Controller 2
#define ENA_2 50
#define ENB_2 51
#define IN1_2 48
#define IN2_2 46
#define IN3_2 47
#define IN4_2 49

#define SPIN 2

Servo servo;
static char *input;
static char *token;

// Tool Data Structures
typedef struct tools
{
    int toolId;
    int status;
    int direction;
    int speed;
} tool;
const int toolCount = 3;
tool myTools[toolCount];

// Input Processing Variables
const byte numChars = 32;
char receivedChars[numChars];
const byte numInts = 4;
int payload[4];
boolean newData = false;

// PWM Processing Variables
#define ON true
#define OFF false
unsigned long currentMicros = micros();
// Controller A (150 us x 100 Count = 15 ms Period / 66 Hz)
unsigned long previousMicros_CA = 0;
unsigned long microInterval_CA = 150;
const byte pwmMax_CA = 100;
// Controller B (1us x 58 Count = 58 us Period / 17.2 kHz)
unsigned long previousMicros_CB = 0;
unsigned long microInterval_CB = 1;
const byte pwmMax_CB = 58;
// PWM Pin Data Structure
typedef struct pwmPins
{
    int pin;          // Pin Number
    int pwmValue;     // Pwm Value (duty cycle)
    bool pinState;    // Pin Output State
    int pwmTickCount; // PWM Counter Value
} pwmPin;

// Motor Controller A - LM298N (Control Brush Motors)
const int pwmPinCount = 2; // Can have maximum 8
const byte pwmPins[pwmPinCount] = {ENA, ENB};
pwmPin myPWMpins[pwmPinCount];

// Motor Controller B - Control DC Motor Wheels
const int pwmPinCount_CB = 4;
const byte pwmPins_CB[pwmPinCount_CB] = {ENA_2, ENB_2};
pwmPin myPWMpins_CB[pwmPinCount_CB];

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {IN1, IN2, IN3, IN4, IN1_2, IN2_2, IN3_2, IN4_2};

// Millis Timer
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long millisInterval = 250;
bool detachServoNextCycle = false;

// Serial Input Processing
void recvWithEndMarker()
{
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && newData == false)
    {
        rc = Serial.read();

        if (rc != endMarker)
        {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars)
            {
                ndx = numChars - 1;
            }
        }
        else
        {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

// Software PWM
void setupPWMpins()
{
    for (int index = 0; index < pwmPinCount; index++)
    {
        myPWMpins[index].pin = pwmPins[index];
        myPWMpins[index].pwmValue = 0;
        myPWMpins[index].pinState = OFF;
        myPWMpins[index].pwmTickCount = 0;
        pinMode(pwmPins[index], OUTPUT);
    }
}

void setupPWMpins_CB()
{
    for (int index = 0; index < pwmPinCount; index++)
    {
        myPWMpins_CB[index].pin = pwmPins_CB[index];
        myPWMpins_CB[index].pwmValue = 0;
        myPWMpins_CB[index].pinState = OFF;
        myPWMpins_CB[index].pwmTickCount = 0;
        pinMode(pwmPins_CB[index], OUTPUT);
    }
}

void handlePWM()
{
    currentMicros = micros();
    // check to see if we need to increment our PWM counters yet
    if (currentMicros - previousMicros_CA >= microInterval_CA) // Counter Period = 150us (100 x 150us = 15 ms Period /66Hz)
    {
        // Increment each pin's counter
        for (int index = 0; index < pwmPinCount; index++)
        {
            myPWMpins[index].pwmTickCount++;

            if (myPWMpins[index].pinState == ON)
            {
                if (myPWMpins[index].pwmTickCount >= myPWMpins[index].pwmValue)
                {
                    myPWMpins[index].pinState = OFF;
                }
            }
            else
            {
                if (myPWMpins[index].pwmTickCount >= pwmMax_CA)
                {
                    myPWMpins[index].pinState = ON;
                    myPWMpins[index].pwmTickCount = 0;
                }
            }
            digitalWrite(myPWMpins[index].pin, myPWMpins[index].pinState);
        }
        // reset the micros() tick counter.
        digitalWrite(13, !digitalRead(13));
        previousMicros_CA = currentMicros;
    }
    if (currentMicros - previousMicros_CB >= microInterval_CB)
    { // Counter Period = 1 us (58 x 1 us = 58 us Period / 17.2 kHz
        // Increment each pin's counter
        for (int index = 0; index < pwmPinCount_CB; index++)
        {
            myPWMpins_CB[index].pwmTickCount++;

            if (myPWMpins_CB[index].pinState == ON)
            {
                if (myPWMpins_CB[index].pwmTickCount >= myPWMpins_CB[index].pwmValue)
                {
                    myPWMpins_CB[index].pinState = OFF;
                }
            }
            else
            {
                if (myPWMpins_CB[index].pwmTickCount >= pwmMax_CA)
                {
                    myPWMpins_CB[index].pinState = ON;
                    myPWMpins_CB[index].pwmTickCount = 0;
                }
            }
            digitalWrite(myPWMpins_CB[index].pin, myPWMpins_CB[index].pinState);
        }
    }
}

// Process Serial Input
void processNewData()
{
    if (newData == true)
    {
        input = receivedChars;
        Serial.println(receivedChars);
        newData = false;
        int counter = 0;
        while ((token = strtok_r(input, ",", &input)))
        {
            payload[counter] = atoi(token);
            counter++;
        }
        handleUpdate();
    }
    newData = false;
}

// Handle Millis Timer - Used to Detach Servos after writing to them
void handleMillis()
{
    currentMillis = millis();
    if (currentMillis - previousMillis >= millisInterval)
    {
        previousMillis = currentMillis;
        if (detachServoNextCycle)
        {
            servo.detach();
            detachServoNextCycle = false;
        }
    }
}

// MOTOR CONTROLLER DIRECTION SETUP FOR LM298N
// DC Motor A
void setDirA1()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
}
void setDirA2()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
}

void brakeA()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
}

// DC Motor B
void setDirB1()
{
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}
void setDirB2()
{
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}
void brakeB()
{
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

// DC Motor A2
void setDirA1_2()
{
    digitalWrite(IN1_2, HIGH);
    digitalWrite(IN2_2, LOW);
}

void setDirA2_2()
{
    digitalWrite(IN1_2, LOW);
    digitalWrite(IN2_2, HIGH);
}

void setDirB1_2()
{
    digitalWrite(IN3_2, HIGH);
    digitalWrite(IN4_2, LOW);
}

void setDirB2_2()
{
    digitalWrite(IN3_2, LOW);
    digitalWrite(IN4_2, HIGH);
}

void onServo()
{
    servo.attach(SPIN);
    servo.write(135);
    detachServoNextCycle = true;
}
void offServo()
{
    servo.attach(SPIN);
    servo.write(179);
    detachServoNextCycle = true;
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
    case 2: // Brush Dc
    {
        myTools[1].status = status;
        myTools[1].speed = speed;
        myTools[1].direction = direction;
        // Set Direction
        myTools[1].status ? myTools[1].direction ? setDirA1() : setDirA2() : brakeA();
        // Update ENA Pin based on status and speed
        myPWMpins[0].pwmValue = myTools[1].status ? myTools[1].speed : 0;
        break;
    }
    case 3: // Pump 1 DC
    {
        myTools[2].status = status;
        myTools[2].speed = speed;
        myTools[2].direction = direction;
        // Set Direction
        myTools[2].status ? myTools[2].direction ? setDirB1() : setDirB2() : brakeB();
        // Update ENB Pin based on status and speed
        myPWMpins[1].pwmValue = myTools[2].status ? myTools[2].speed : 0;
        break;
    }
    case 4: // Wheel Controller 1 - Motor Controller 2
    {
        if (myTools[3].status && !status)
        {
            // Brake
            digitalWrite(IN1_2, LOW);
            digitalWrite(IN2_2, LOW);
            myTools[3].status = 0;
        }
        myTools[3].status = status;
        myTools[3].speed = speed;
        if (myTools[3].status)
        {
            if (direction != myTools[3].direction)
            {
                myTools[3].direction = direction;
                digitalWrite(IN1_2, LOW);
                digitalWrite(IN2_2, LOW);
            }

            myTools[3].direction == 1 ? setDirA1_2() : setDirA2_2();
        }
        // Update EN Pin based on status and speed
        myPWMpins_CB[0].pwmValue = myTools[3].status ? myTools[3].speed : 0;
        break;
    }
    case 5:
    {
        if (myTools[4].status && !status)
        {
            // Brake
            digitalWrite(IN3_2, LOW);
            digitalWrite(IN4_2, LOW);
            myTools[4].status = 0;
        }
        myTools[4].status = status;
        myTools[4].speed = speed;
        if (myTools[4].status)
        {
            if (direction != myTools[4].direction)
            {
                myTools[4].direction = direction;
                digitalWrite(IN3_2, LOW);
                digitalWrite(IN4_2, LOW);
            }

            myTools[4].direction == 1 ? setDirB1_2() : setDirB2_2();
        }
        // Update EN Pin based on status and speed
        myPWMpins_CB[1].pwmValue = myTools[4].status ? myTools[4].speed : 0;
        break;
    }
    default:
    {
        Serial.println("[ERROR] Invalid Tool ID - " + payload[0]);
        break;
    }
    }
    Serial.println("[DONE]");
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("<Arduino is ready>");
    delay(1000);
    for (int index = 0; index < digitalPinCount; index++)
    {
        pinMode(digitalPins[index], OUTPUT);
    }
    setupPWMpins();
    setupPWMpins_CB();
}

void loop()
{
    handlePWM();
    handleMillis();
    recvWithEndMarker();
    processNewData();
}