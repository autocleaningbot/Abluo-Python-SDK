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
#define ENA 3
#define ENB 8
#define IN1 4
#define IN2 5
#define IN3 6
#define IN4 7

// Motor Controller 2
#define ENA_2 32
#define ENB_2 30
#define IN1_2 28
#define IN2_2 26
#define IN3_2 24
#define IN4_2 22

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
unsigned long previousMicros = 0;
unsigned long microInterval = 150;
const byte pwmMax = 100;

typedef struct pwmPins
{
    int pin;          // Pin Number
    int pwmValue;     // Pwm Value (duty cycle)
    bool pinState;    // Pin Output State
    int pwmTickCount; // PWM Counter Value
} pwmPin;

const int pwmPinCount = 4; //Can have maximum 8
const byte pwmPins[pwmPinCount] = {ENA, ENB, ENA_2, ENB_2};
pwmPin myPWMpins[pwmPinCount];

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

void handlePWM()
{
    currentMicros = micros();
    // check to see if we need to increment our PWM counters yet
    if (currentMicros - previousMicros >= microInterval)
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
                if (myPWMpins[index].pwmTickCount >= pwmMax)
                {
                    myPWMpins[index].pinState = ON;
                    myPWMpins[index].pwmTickCount = 0;
                }
            }
            digitalWrite(myPWMpins[index].pin, myPWMpins[index].pinState);
        }
        // reset the micros() tick counter.
        digitalWrite(13, !digitalRead(13));
        previousMicros = currentMicros;
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
        myTools[1].direction == 1 ? setDirA1() : setDirA2();
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
        myTools[2].direction == 1 ? setDirB1() : setDirB2();
        // Update ENB Pin based on status and speed
        myPWMpins[1].pwmValue = myTools[2].status ? myTools[2].speed : 0;
        break;
    }
    case 4: // Pump 2 DC
        myTools[3].status = status;
        myTools[3].speed = speed;
        myTools[3].direction = direction;
        // Set Direction
        myTools[3].direction == 1 ? setDirB1() : setDirB2();
        // Update ENB Pin based on status and speed
        myPWMpins[2].pwmValue = myTools[3].status ? myTools[3].speed : 0;
    {
        break;
    }
    default:
        Serial.println("[ERROR] Invalid Tool ID - " + payload[0]);
        break;
    }
    Serial.println("[DONE]");
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("<Arduino is ready>");
    delay(1000);
    for (int index = 0; index < digitalPinCount; index++){
        pinMode(digitalPins[index], OUTPUT);
    }
    setupPWMpins();
}

void loop()
{
    handlePWM();
    handleMillis();
    recvWithEndMarker();
    processNewData();
}