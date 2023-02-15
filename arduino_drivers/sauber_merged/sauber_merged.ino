/***
   Arduino Code: Command movement of 4 meccanum wheels and get encoder data using a single Arduino Mega
   @file abluo_base_merged.ino
   @author Wang Huachen (huachenw24@gmail.com)
   @references: https://www.baldengineer.com/software-pwm-with-millis.html
   @version: 2.0 (27 December 2022)

  * Takes in I2C Input of the form <fl, fr, bl, br>
  * Updates Data Model to store MotorState
  * Executes Software PWM to control multiple motors handling Motors

*/
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <avr/dtostrf.h>
#include "src/I2C_PROCESSING.h"
#include <Encoder.h>

// Front Left Wheel - DC Controller 1 Motor 1
#define FL_EN 2
#define FL_IN1 3
#define FL_IN2 4

// Front Right Wheel - DC Controller 1 Motor 2
#define FR_EN 5
#define FR_IN1 7
#define FR_IN2 6

// Back Left Wheel - DC Controller 2 Motor 1
#define BL_EN 8
#define BL_IN1 9
#define BL_IN2 10

// Back Right Wheel - DC Controller 2 Motor 2
#define BR_EN 11
#define BR_IN1 13
#define BR_IN2 12

#define ESTOP_INT 22

// Encoder input pins (only 1 needs to be interrupt)
Encoder fl_Enc(54, 55);
Encoder fr_Enc(57, 56);
Encoder bl_Enc(58, 59);
Encoder br_Enc(61, 60);

// Tick count for 1 rev
#define ENC_FL_REV 100
#define ENC_FR_REV 100
#define ENC_BL_REV 100
#define ENC_BR_REV 100

// Helpers
bool settingUp = true;
bool emergency = false;

// Constants
const float twopi = 6.283185307; // 2pi

// Variables
long prevMicros = micros();
long prevMillis = millis();
float ang_vel_FL = 0;
float ang_vel_FR = 0;
float ang_vel_BL = 0;
float ang_vel_BR = 0;
char padded_FL[8];
char padded_FR[8];
char padded_BL[8];
char padded_BR[8];
char speedChars[32];

// Enums for Wheel Index and Direction
enum WHEELS_INDEX
{
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  BACK_LEFT = 2,
  BACK_RIGHT = 3
};

// Motor PWM Pins
const byte motorPinsCount = 4;
const byte motorPwmPins[motorPinsCount] = {FL_EN, FR_EN, BL_EN, BR_EN};

// Motor Direction Pins
const byte digitalMotorPinCount = 8;
const byte digitalMotorPins[digitalMotorPinCount] = {FL_IN1, FL_IN2, FR_IN1, FR_IN2, BL_IN1, BL_IN2, BR_IN1, BR_IN2};

// WHEELS DIRECTION & BRAKE
void move_FL(int duty)
{
  if (duty == 0) {
    digitalWrite(FL_IN1, LOW);
    digitalWrite(FL_IN2, LOW);
    analogWrite(FL_EN, 0);
  } else if (duty > 0) {
    digitalWrite(FL_IN1, HIGH);
    digitalWrite(FL_IN2, LOW);
    analogWrite(FL_EN, duty * 0.01 * 255);
  } else if (duty < 0) {
    digitalWrite(FL_IN1, LOW);
    digitalWrite(FL_IN2, HIGH);
    analogWrite(FL_EN, duty * -0.01 * 255);
  }
}

void move_FR(int duty)
{
  if (duty == 0) {
    digitalWrite(FR_IN1, LOW);
    digitalWrite(FR_IN2, LOW);
    analogWrite(FR_EN, 0);
  } else if (duty > 0) {
    digitalWrite(FR_IN1, HIGH);
    digitalWrite(FR_IN2, LOW);
    analogWrite(FR_EN, duty * 0.01 * 255);
  } else if (duty < 0) {
    digitalWrite(FR_IN1, LOW);
    digitalWrite(FR_IN2, HIGH);
    analogWrite(FR_EN, duty * -0.01 * 255);
  }
}

void move_BL(int duty)
{
  if (duty == 0) {
    digitalWrite(BL_IN1, LOW);
    digitalWrite(BL_IN2, LOW);
    analogWrite(BL_EN, 0);
  } else if (duty > 0) {
    digitalWrite(BL_IN1, HIGH);
    digitalWrite(BL_IN2, LOW);
    analogWrite(BL_EN, duty * 0.01 * 255);
  } else if (duty < 0) {
    digitalWrite(BL_IN1, LOW);
    digitalWrite(BL_IN2, HIGH);
    analogWrite(BL_EN, duty * -0.01 * 255);
  }
}

void move_BR(int duty)
{
  if (duty == 0) {
    digitalWrite(BR_IN1, LOW);
    digitalWrite(BR_IN2, LOW);
    analogWrite(BR_EN, 0);
  } else if (duty > 0) {
    digitalWrite(BR_IN1, HIGH);
    digitalWrite(BR_IN2, LOW);
    analogWrite(BR_EN, duty * 0.01 * 255);
  } else if (duty < 0) {
    digitalWrite(BR_IN1, LOW);
    digitalWrite(BR_IN2, HIGH);
    analogWrite(BR_EN, duty * -0.01 * 255);
  }
}

void stopAllMotors()
{
  move_FL(0);
  move_FR(0);
  move_BL(0);
  move_BR(0);
}

/**
 * @brief Calls respective handler based on command
 *
 */
void handleCommand()
{
  move_FL(payload[0]);
  move_FR(payload[1]);
  move_BL(payload[2]);
  move_BR(payload[3]);
}

/**
 * @brief Checks for new serial inputs and calls command handler
 *
 */
void processNewData()
{
  if (newData == true)
  {
    parseInput();
    handleCommand();
  }
  newData = false;
}

void readEncoders()
{
  fl_Enc.read();
  fr_Enc.read();
  bl_Enc.read();
  br_Enc.read();
}

void requestEvent()
{
  long currMillis = millis();
  float deltaMillis = (float)(currMillis - prevMillis);

  long posFL = fl_Enc.read();
  long posFR = fr_Enc.read();
  long posBL = bl_Enc.read();
  long posBR = br_Enc.read();

  ang_vel_FL = (float)(posFL * twopi / deltaMillis * 1000 / ENC_FL_REV);
  ang_vel_FR = (float)(posFR * twopi / deltaMillis * 1000 / ENC_FR_REV);
  ang_vel_BL = (float)(posBL * twopi / deltaMillis * 1000 / ENC_BL_REV);
  ang_vel_BR = (float)(posBR * twopi / deltaMillis * 1000 / ENC_BR_REV);

  speedChars[0] = 0;
  dtostrf(ang_vel_FL, 7, 2, padded_FL);
  strcat(speedChars, padded_FL);
  strcat(speedChars, ",");
  dtostrf(ang_vel_FR, 7, 2, padded_FR);
  strcat(speedChars, padded_FR);
  strcat(speedChars, ",");
  dtostrf(ang_vel_BL, 7, 2, padded_BL);
  strcat(speedChars, padded_BL);
  strcat(speedChars, ",");
  dtostrf(ang_vel_BR, 7, 2, padded_BR);
  strcat(speedChars, padded_BR);

  fl_Enc.write(0);
  fr_Enc.write(0);
  bl_Enc.write(0);
  br_Enc.write(0);

  Wire.write(speedChars, 31);
  prevMillis = millis();
}

void setup()
{
  // Begin I2C
  Wire.begin(0x70);
  Wire.setClock(400000);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  // Set 5V and GND Pins
  pinMode(ESTOP_INT, INPUT_PULLUP);

  // Setup Enable Pins
  for (int index = 0; index < motorPinsCount; index++)
  {
    pinMode(motorPwmPins[index], OUTPUT);
  }
  
  // Setup Motor Pins
  for (int index = 0; index < digitalMotorPinCount; index++)
  {
    pinMode(digitalMotorPins[index], OUTPUT);
  }
  
  settingUp = false;
}

void loop()
{
  if (digitalRead(ESTOP_INT) == LOW)
  {
    emergency = true;
    stopAllMotors();
  } 
  else
  {
    emergency = false;
  }
  if (!settingUp)
  {
    if (!emergency)
    {
      readEncoders();
      processNewData();
    }
    else
    {
      parseInput();
    }
  }
}
