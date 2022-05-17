////////////////////////////////
// This is a demo program for the rotary grabber
//--connect the servo motor signal wire to pin 8
//--open serial monitor with 9600 baud rate
//--key in "1" enter, to lock the brush
//--key in "0" enter, to release the brush
////////////////////////////////////////////

#include <Servo.h>

#define ENA 3
#define IN1 4
#define IN2 5
#define IN3 6
#define ENB 7
#define SPIN 2
Servo servo;
char input;
String readString;
bool dcOnA = false;
int dutyCycleA = 0;
int directionA = 0;

// Input Processing Variables
const byte numChars = 32;
char receivedChars[numChars];
const byte numInts = 4;
int payload[4];
boolean newData = false;

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

void showNewData()
{
    if (newData == true)
    {
        Serial.print("This just in ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("<Arduino is ready>");
    delay(1000);
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
}

void runPWMA()
{
    if (dcOnA)
    {
        digitalWrite(ENA, HIGH);
        delayMicroseconds(dutyCycleA * 200);
        digitalWrite(ENA, LOW);
        delayMicroseconds((100 - dutyCycleA) * 200);
    } else {
        digitalWrite(ENA, LOW);
    }
    return;
}

// RUN DC MOTOR AT 50 HZ WITH PWM
void runDc()
{
    runPWMA();
}

// MOTOR CONTROLLER DIRECTION SETUP FOR LM298N
void setDcClockwise()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
}

void setDcAntiClockwise()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
}

void onServo()
{
    servo.attach(SPIN);
    servo.write(155);
    delay(250);
    servo.detach();
}
void offServo()
{
    servo.attach(SPIN);
    servo.write(179);
    delay(250);
    servo.detach();
}

void handleUpdate()
{
    int status = payload[1];
    int direction = payload[2];
    int speed = payload[3];
    switch (payload[0])
    {
    // Brush Servo
    case 1:
    {
        status == 1 ? onServo() : offServo();
        break;
    }
    // Brush Dc
    case 2:
    {
        dcOnA = status;
        dutyCycleA = speed;
        directionA = direction;
        directionA == 1 ? setDcAntiClockwise() : setDcClockwise();
        break;
    }
    // Pump 1 DC
    case 3:
    {
        break;
    }
    // Pump 2 DC
    case 4:
    {
        break;
    }
    default:
        Serial.println("[ERROR] Invalid Tool ID - " + payload[1]);
        break;
    }
    Serial.println("[DONE]");
}

void processNewData()
{
    if (newData == true)
    {
        char *input = receivedChars;
        Serial.println(receivedChars);
        newData = false;
        char *token;
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

void loop()
{
    recvWithEndMarker();
    processNewData();
    runDc(); // Run DC Motor PWM Loop
    // showNewData();
    // if (Serial.available() > 0){
    //     String in = Serial.readString();
    //     Serial.println(in);
    // }
}

// void loop()
// {
//     if (Serial.available() > 0)
//     {
//         input = Serial.read();
//         Serial.println(input);
//         if (input == '1')
//         {
//             onServo();
//             startDcClockwise();
//         }
//         else if (input == '2')
//         {
//             onServo();
//             startDcAntiClockwise();
//         }
//         else if (input == '0')
//         {
//             stopDc();
//             offServo();
//         }
//     }
//     runDc();
// }