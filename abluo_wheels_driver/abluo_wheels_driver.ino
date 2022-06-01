/***
   @file abluo_Motors.ino
   @brief Abluo Motor Control Driver for Arduino Mega

   Takes in Serial Input of the form <motorId,status,direction,speed>
   Updates Data Model to store MotorState
   Executes Software PWM to control multiple motors handling Motors

    Current Motors         |           Valid Params            |                   Description                         |
   --------------------------------------------------------------------------------------------------------------
   1. M0  - Brush Servo   | <motorId,status>                   | status: 1 - Lock, 0: Unlock
   2. M1A - Brush DC      | <motorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
   3. M1B - Water Pump DC | <motorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100
   4. M2A - Wheel 1       | <motorId,status,direction,speed>   | status: 1- ON, 0: OFF, direction: 1,0, speed: 0 - 100

   @author Rishab Patwari (patwariri@gmail.com)
   @references: https://www.baldengineer.com/software-pwm-with-millis.html
*/
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
#define M2_IN1 46
#define M2_IN2 48
// DC Controller 2 - Motor 2
#define M2B_EN 51
#define M2_IN3 49
#define M2_IN4 47
// #define M2B_EN 9
// #define M2_IN3 10
// #define M2_IN4 11

// Servo Controller Pin
#define SPIN 2
#define SPEED_LIMIT 25

bool settingUp = true;
enum WHEELS_INDEX
{
  FRONT_LEFT = 0,
  FRONT_RIGHT = 1,
  BACK_LEFT = 2,
  BACK_RIGHT = 3
};
enum DIRECTION
{
  FORWARD = 0,
  BACKWARD = 1,
  LEFT = 2,
  RIGHT = 3,
  FORWARD_RIGHT = 4,
  FORWARD_LEFT = 5,
  BACKWARD_RIGHT = 6,
  BACKWARD_LEFT = 7,
  CLOCKWISE = 8,
  ANTI_CLOCKWISE = 9
};
unsigned long motion_duration;
unsigned long motion_start_millis;

typedef void (*functiontype)();
unsigned long currentMicros = micros();

// Controller M2: Motor PWM Controller
// Frequency: (1us x 58 Count = 58 us Period / 17.2 kHz)
const byte motorMicroInterval = 1;
const byte motorPwmMax = 58;
const byte motorPinsCount = 4;
const byte motorPwmPins[motorPinsCount] =
    {
        M1A_EN,
        M1B_EN, M2A_EN, M2B_EN};
PWM_PROCESSING pwmMotorsController;

// Motor Data Structures
struct Motor
{
  int motorId;
  int status;
  int direction;
  int speed;
};
const byte MotorCount = 4;
Motor myMotors[MotorCount];

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {
    M1_IN1,
    M1_IN2,
    M1_IN3,
    M1_IN4,
    M2_IN1,
    M2_IN2,
    M2_IN3,
    M2_IN4};

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

void setupMotors()
{
  for (int index = 1; index <= MotorCount; index++)
  {
    myMotors->motorId = index;
    myMotors->status = 0;
    myMotors->direction = 0;
    myMotors->speed = 0;
  }
}

void handleDcMotor(int motorIndex, int status, int direction, int speed, functiontype setDir_1, functiontype setDir_2, functiontype brake)
{
  myMotors[motorIndex].status = status;
  myMotors[motorIndex].speed = speed;
  myMotors[motorIndex].direction = direction;
  if (myMotors[motorIndex].status)
  {
    myMotors[motorIndex].direction == 1 ? setDir_1() : setDir_2();
    pwmMotorsController.updatePinPwmDutyCycle(motorIndex, speed);
  }
  else
  {
    brake();
  }
  Serial.println("[DONE]");
}

void stopAllMotors()
{
  for (int i = 0; i < MotorCount; i++)
  {
    myMotors[i].status = 0;
  }
  brake_M1A();
  brake_M1B();
  brake_M2A();
  brake_M2B();
}

void handleMotorMotion()
{
  if (motion_duration > 0)
  {
    currentMillis = millis();
    if (currentMillis - motion_start_millis > motion_duration)
    {
      char buffer[40];
      sprintf(buffer, "[Motion Completed] - Duration = %d", motion_duration);
      motion_duration = 0;
      stopAllMotors();
      Serial.println(buffer);
    }
  }
}

void handleUpdate()
{
  int motorId = payload[0];
  int motorIndex = payload[0] - 1;
  int status = payload[1];
  int direction = payload[2];
  int speed = payload[3];
  if (speed > SPEED_LIMIT)
  {
    speed = SPEED_LIMIT;
  }
  switch (motorId)
  {
  case 1: // M1_A - Front Left
  {
    handleDcMotor(motorIndex, status, direction, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    break;
  }
  case 2: // M1_B - Front Right
  {

    handleDcMotor(motorIndex, status, direction, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    break;
  }
  case 3: // M2_A - Back Left
  {
    handleDcMotor(motorIndex, status, direction, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    break;
  }
  case 4: // M2_B - Back Right
  {
    handleDcMotor(motorIndex, status, direction, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    break;
  }
  case 5: // Move Forward or Back
  {
    int direction = payload[1];
    int duration = payload[2];
    int speed = payload[3];
    motion_duration = duration;
    motion_start_millis = millis();
    if (speed > SPEED_LIMIT)
    {
      speed = SPEED_LIMIT;
    }

    if (direction == DIRECTION::FORWARD)
    { // Forward
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    if (direction == DIRECTION::BACKWARD)
    { // BACKWARD
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    if (direction == DIRECTION::RIGHT)
    { // RIGHT
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    if (direction == DIRECTION::LEFT)
    { // LEFT
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    if (direction == DIRECTION::CLOCKWISE)
    {
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    if (direction == DIRECTION::ANTI_CLOCKWISE)
    {
      handleDcMotor(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      handleDcMotor(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      handleDcMotor(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      handleDcMotor(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
    }
    break;
  }
  case 6:
  {
    int direction = payload[1];
    int duration = payload[2];
    int speed = payload[3];
    stopAllMotors();
    Serial.println("[STOPPED ALL MOTORS]");
    break;
  }
  default:
  {
    char buffer[40];
    sprintf(buffer, "[ERROR] Invalid Motor ID - %d", payload[0]);
    Serial.println(buffer);
    break;
  }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  for (int index = 0; index < digitalPinCount; index++)
  {
    pinMode(digitalPins[index], OUTPUT);
  }
  pwmMotorsController.init(motorMicroInterval, motorPwmMax, motorPwmPins, motorPinsCount);
  Serial.println("<Arduino is ready>");
  settingUp = false;
}

void loop()
{
  if (!settingUp)
  {
    recvWithEndMarker();
    processNewData();
    pwmMotorsController.handlePWM();
    handleMotorMotion();
  }
}
