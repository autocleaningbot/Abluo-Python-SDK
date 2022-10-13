/***
   Abluo Wheels Driver: Driver to command 4 wheel meccanum wheel robot (without encoder feedback)
   @file abluo_wheels_driver.ino
   @author Rishab Patwari (patwariri@gmail.com)
   @references: https://www.baldengineer.com/software-pwm-with-millis.html
   @version: 1.0 (1 June 2022)

  * Takes in Serial Input of the form <commandId,motionDirection,speed,duration(optional)>
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
#include "src/SERIAL_PROCESSING.h"
#include "src/PWM_PROCESSING.h"

#define BAUD_RATE 115200
// DC Controller 1 - Motor 1
#define M1A_EN 9
#define M1_IN1 48
#define M1_IN2 49

// DC Controller 1 - Motor 2
#define M1B_EN 13
#define M1_IN3 47
#define M1_IN4 43

// DC Controller 2 - Motor 1
#define M2A_EN 10
#define M2_IN1 12
#define M2_IN2 11

// DC Controller 2 - Motor 2
#define M2B_EN 6
#define M2_IN3 7
#define M2_IN4 8

#define SPEED_LIMIT 25

// Helpers
bool settingUp = true;
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
const byte motorPwmPins[motorPinsCount] = {M1A_EN, M1B_EN, M2A_EN, M2B_EN};
PWM_PROCESSING pwmMotorsController;

// Digital Pins (Pins not used for Software PWM Generation)
const byte digitalPinCount = 8;
const byte digitalPins[digitalPinCount] = {M1_IN1, M1_IN2, M1_IN3, M1_IN4, M2_IN1, M2_IN2, M2_IN3, M2_IN4};

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
 * @param setDir_1 function to set direction 1 of motor
 * @param setDir_2 function to set direction 2 of motor
 * @param brake function to brake and stop motor
 */
void updateDcMotorState(int motorIndex, int status, int direction, int speed, functiontype setDir_1, functiontype setDir_2, functiontype brake)
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
  for (int i = 0; i < motorCount; i++)
  {
    myMotors[i].status = 0;
  }
  brake_M1A();
  brake_M1B();
  brake_M2A();
  brake_M2B();
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
      char buffer[40];
      sprintf(buffer, "[Motion Completed] - Duration = %d", motion_duration);
      motion_duration = 0;
      stopAllMotors();
      Serial.println(buffer);
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
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
  if (direction == DIRECTION::BACKWARD)
  { // BACKWARD
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
  if (direction == DIRECTION::RIGHT)
  { // RIGHT
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
  if (direction == DIRECTION::LEFT)
  { // LEFT
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
  if (direction == DIRECTION::CLOCKWISE)
  {
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::FORWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::FORWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::BACKWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
  if (direction == DIRECTION::ANTI_CLOCKWISE)
  {
    updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, 1, DIRECTION::BACKWARD, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
    updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, 1, DIRECTION::FORWARD, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
    updateDcMotorState(WHEELS_INDEX::BACK_LEFT, 1, DIRECTION::BACKWARD, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
    updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, 1, DIRECTION::FORWARD, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
  }
}

/**
 * @brief Brakes and stops all motors
 *
 */
void handleStop()
{
  stopAllMotors();
  Serial.println("[STOPPED ALL MOTORS]");
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
      updateDcMotorState(WHEELS_INDEX::FRONT_LEFT, status, wheelDirection, speed, set_M1A_D1, set_M1A_D2, brake_M1A);
      break;
    }
    case 1: // M1_B - Front Right
    {
      updateDcMotorState(WHEELS_INDEX::FRONT_RIGHT, status, wheelDirection, speed, set_M1B_D1, set_M1B_D2, brake_M1B);
      break;
    }
    case 2: // M2_A - Back Left
    {
      updateDcMotorState(WHEELS_INDEX::BACK_LEFT, status, wheelDirection, speed, set_M2A_D1, set_M2A_D2, brake_M2A);
      break;
    }
    case 3: // M2_B - Back Right
    {
      updateDcMotorState(WHEELS_INDEX::BACK_RIGHT, status, wheelDirection, speed, set_M2B_D1, set_M2B_D2, brake_M2B);
      break;
    }
    };
    break;
  }
  default:
  {
    char buffer[40];
    sprintf(buffer, "[ERROR] Invalid Command ID - %d", payload[0]);
    Serial.println(buffer);
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

void setup()
{
  // Begin Serial and Setup Pins
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
    handleDurationBasedMotorMotion();
  }
}
