/***
   Abluo Wheels Driver: Driver to command 4 wheel meccanum wheel robot (without encoder feedback)
   @file wheels_i2c.ino
   @author Rishab Patwari (patwariri@gmail.com) & Wang Huachen (huachenw24@gmail.com)
   @references: https://www.baldengineer.com/software-pwm-with-millis.html
   @version: 1.0 (11 October 2022)

  * Takes in I2C Input of the form <commandId,motionDirection,speed,duration(optional)>
  * Updates Data Model to store MotorState
  * Executes Software PWM to control multiple motors handling Motors

  Command   |           Valid Params                        |                   Description
  --------------------------------------------------------------------------------------------------------------
    1       | <commandId,motionDirection, speed, duration > | Duration Based Movement in set Direction
    2       | <commandId,motionDirection, speed >           | Continuous Movement in set Direction
    3       | <commandId >                                  | Stop Movement

  Direction Mapping
  -------------------
  0 : FORWARD
  1 : BACKWARD
  2 : LEFT
  3 : RIGHT
  8 : CLOCKWISE
  9 : ANTI-CLOCKWISE
*/
#include <Wire.h>
#include "src/I2C_PROCESSING.h"
#include "src/PWM_PROCESSING.h"

// Front Left Wheel - DC Controller 1 Motor 1
#define FL_EN 9
#define FL_IN1 49
#define FL_IN2 48

// Front Right Wheel - DC Controller 1 Motor 2
#define FR_EN 13
#define FR_IN1 43
#define FR_IN2 47

// Back Left Wheel - DC Controller 2 Motor 1
#define BL_EN 10
#define BL_IN1 11
#define BL_IN2 12

// Back Right Wheel - DC Controller 2 Motor 2
#define BR_EN 6
#define BR_IN1 8
#define BR_IN2 7

#define SPEED_LIMIT 25
#define ESTOP_INT 3

// Helpers
bool settingUp = true;
bool emergency = false;
typedef void (*functiontype)();
unsigned long currentMicros = micros();
unsigned long currentMillis = millis();
unsigned long motion_duration;
unsigned long motion_start_millis;

// Enums for Wheel Index and Direction
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

// Software PWM
// Frequency: (1us x 58 Count = 58 us Period / 17.2 kHz)
const byte motorMicroInterval = 1;
const byte motorPwmMax = 58;
const byte motorPinsCount = 4;
const byte motorPwmPins[motorPinsCount] = {FL_EN, FR_EN, BL_EN, BR_EN};
PWM_PROCESSING pwmMotorsController;

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {FL_IN1, FL_IN2, FR_IN1, FR_IN2, BL_IN1, BL_IN2, BR_IN1, BR_IN2};

// Motor Data Structures
struct Motor
{
  int motorId;
  int status;
  int direction;
  int speed;
};
const byte motorCount = 4;
Motor myMotors[motorCount];

// WHEELS DIRECTION & BRAKE
// FRONT LEFT (MC1A)
void set_FL_Forward()
{
  digitalWrite(FL_IN1, HIGH);
  digitalWrite(FL_IN2, LOW);
}
void set_FL_Backward()
{
  digitalWrite(FL_IN1, LOW);
  digitalWrite(FL_IN2, HIGH);
}
void brake_FL()
{
  digitalWrite(FL_IN1, LOW);
  digitalWrite(FL_IN2, LOW);
}

// FRONT RIGHT (MC1B)
void set_FR_Forward()
{
  digitalWrite(FR_IN1, HIGH);
  digitalWrite(FR_IN2, LOW);
}
void set_FR_Backward()
{
  digitalWrite(FR_IN1, LOW);
  digitalWrite(FR_IN2, HIGH);
}
void brake_FR()
{
  digitalWrite(FR_IN1, LOW);
  digitalWrite(FR_IN2, LOW);
}

// BACK LEFT (MC2A)
void set_BL_Forward()
{
  digitalWrite(BL_IN1, HIGH);
  digitalWrite(BL_IN2, LOW);
}
void set_BL_Backward()
{
  digitalWrite(BL_IN1, LOW);
  digitalWrite(BL_IN2, HIGH);
}
void brake_BL()
{
  digitalWrite(BL_IN1, LOW);
  digitalWrite(BL_IN2, LOW);
}

// BACK RIGHT (MC2B)
void set_BR_Forward()
{
  digitalWrite(BR_IN1, HIGH);
  digitalWrite(BR_IN2, LOW);
}
void set_BR_Backward()
{
  digitalWrite(BR_IN1, LOW);
  digitalWrite(BR_IN2, HIGH);
}
void brake_BR()
{
  digitalWrite(BR_IN1, LOW);
  digitalWrite(BR_IN2, LOW);
}

void setupMotors()
{
  for (int index = 1; index <= motorCount; index++)
  {
    myMotors->motorId = index;
    myMotors->status = 0;
    myMotors->direction = 0;
    myMotors->speed = 0;
  }
}

/**
 * @brief Updates Direction & PWM Duty Cycle for Motor
 *
 * @param motorIndex index of motor to be updated
 * @param status on or off status of the motor
 * @param direction desired direction of spin of the motor
 * @param speed desired pwm duty cycle
 * @param setForward function to set direction 1 of motor
 * @param setBackward function to set direction 2 of motor
 * @param brake function to brake and stop motor
 */
void updateDcMotorState(int motorIndex, int status, int direction, int speed, functiontype setForward, functiontype setBackward, functiontype brake)
{
  myMotors[motorIndex].status = status;
  myMotors[motorIndex].speed = speed;
  myMotors[motorIndex].direction = direction;
  if (myMotors[motorIndex].status)
  {
    myMotors[motorIndex].direction == 0 ? setForward() : setBackward();
    pwmMotorsController.updatePinPwmDutyCycle(motorIndex, speed);
  }
  else
  {
    brake();
  }
}

void stopAllMotors()
{
  for (int i = 0; i < motorCount; i++)
  {
    myMotors[i].status = 0;
  }
  brake_FL();
  brake_FR();
  brake_BL();
  brake_BR();
}

/**
 * @brief Stops all motors when duration based motor motion is complete
 *
 */
void handleDurationBasedMotorMotion()
{
  if (motion_duration > 0)
  {
    currentMillis = millis();
    if (currentMillis - motion_start_millis > motion_duration)
    {
      motion_duration = 0;
      stopAllMotors();
    }
  }
}

/**
 * @brief Updates motor state corresponding to desired direction of movement
 *
 * @param direction desired direction of movement of robot
 * @param speed desired pwm duty cycle for all motors (same for every motor)
 */
void handleMovement(int direction, int speed)
{
  // Constraint Speed to Speed Limit
  if (speed > SPEED_LIMIT)
    speed = SPEED_LIMIT;

  // Update Motor Desired State based on Desired Direction
  if (direction == DIRECTION::FORWARD)
  { // Forward
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
  if (direction == DIRECTION::BACKWARD)
  { // BACKWARD
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
  if (direction == DIRECTION::RIGHT)
  { // RIGHT
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
  if (direction == DIRECTION::LEFT)
  { // LEFT
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
  if (direction == DIRECTION::CLOCKWISE)
  {
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
  if (direction == DIRECTION::ANTI_CLOCKWISE)
  {
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_FL_Forward, set_FL_Backward, brake_FL);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_FR_Forward, set_FR_Backward, brake_FR);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_BL_Forward, set_BL_Backward, brake_BL);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_BR_Forward, set_BR_Backward, brake_BR);
  }
}

/**
 * @brief Brakes and stops all motors
 *
 */
void handleStop()
{
  stopAllMotors();
}

/**
 * @brief Calls respective handler based on command
 *
 */
void handleCommand()
{
  int commandId = payload[0];
  switch (commandId)
  {
  case 1: // Movement with duration
  {
    int motionDirection = payload[1];
    int speed = payload[2];
    int duration = payload[3];
    // Setting motion_duration auto stops wheels once duration is complete
    motion_duration = duration;
    motion_start_millis = millis();
    handleMovement(motionDirection, speed);
    break;
  }
  case 2: // Continous Movement
  {
    int motionDirection = payload[1];
    int speed = payload[2];
    handleMovement(motionDirection, speed);
    break;
  }
  case 3: // Stop
  {
    handleStop();
    break;
  }
  case 4: // Set Direction (0: Forward, 1: Backward) and Speed of Particular Motor
  {
    int wheelDirection = payload[1];
    int speed = payload[2];
    int motorIndex = payload[3];
    int status = payload[4];
    switch (motorIndex)
    {
    case 0: // M1_A - Front Left
    {
      updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, status, wheelDirection, speed, set_FL_Forward, set_FL_Backward, brake_FL);
      break;
    }
    case 1: // M1_B - Front Right
    {
      updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, status, wheelDirection, speed, set_FR_Forward, set_FR_Backward, brake_FR);
      break;
    }
    case 2: // M2_A - Back Left
    {
      updateDcMotorState(WHEELS_INDEX::BACK_LEFT, status, wheelDirection, speed, set_BL_Forward, set_BL_Backward, brake_BL);
      break;
    }
    case 3: // M2_B - Back Right
    {
      updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, status, wheelDirection, speed, set_BR_Forward, set_BR_Backward, brake_BR);
      break;
    }
    }
  }
  default:
  {
    break;
  }
  }
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

void emergencyStop()
{
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 500)
  {
    if (digitalRead(ESTOP_INT) == HIGH)
    {
      emergency = true;
      stopAllMotors();
      else
      {
        emergency = false;
      }
    }
  }
}

void setup()
{
  // Begin I2C and Setup Pins
  Wire.begin(0x70);
  Wire.onReceive(receiveEvent);
  for (int index = 0; index < digitalPinCount; index++)
  {
    pinMode(digitalPins[index], OUTPUT);
  }
  pinMode(ESTOP_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ESTOP_INT), emergencyStop, CHANGE);
  pwmMotorsController.init(motorMicroInterval, motorPwmMax, motorPwmPins, motorPinsCount);
  settingUp = false;
}

void loop()
{
  if (!settingUp && !emergency)
  {
    processNewData();
    pwmMotorsController.handlePWM();
    handleDurationBasedMotorMotion();
  }
}
