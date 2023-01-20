#include <Servo.h> 
#include "src/I2C_PROCESSING.h"

#define MOTOR_EN 9
#define MOTOR_IN1 7
#define MOTOR_IN2 8
#define SERVO 3
#define ESTOP_INT 5

bool settingUp = true;
bool emergency = false;

const byte pwmPinCount = 2;
const byte pwmPins[pwmPinCount] = {MOTOR_EN, SERVO};

const byte digitalPinCount = 2;
const byte digitalPins[digitalPinCount] = {MOTOR_IN1, MOTOR_IN2};

Servo BrushServo;

void move_MOTOR(int duty)
{
  if (duty == 0) {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, LOW);
    analogWrite(MOTOR_EN, 0);
  } else if (duty > 0) {
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    analogWrite(MOTOR_EN, duty * 0.01 * 255);
  } else if (duty < 0) {
    digitalWrite(MOTOR_IN1, LOW);
    digitalWrite(MOTOR_IN2, HIGH);
    analogWrite(MOTOR_EN, duty * -0.01 * 255);
  }
}

void move_SERVO(int angle)
{
  BrushServo.write(angle);
}

void stopAllMotors()
{
  move_MOTOR(0);
}

void handleCommand()
{
  int toolSelect = payload[0];
  if (toolSelect == 0) {
    int motorPWM = payload[1];
    move_MOTOR(motorPWM);
  } else if (toolSelect == 1) {
    int servoAngle = payload[1];
    move_SERVO(servoAngle);
  }
}

void processNewData()
{
  if (newData == true)
  {
    parseInput();
    handleCommand();
  }
  newData = false;
}

void setup() {
  Wire.begin(0x60);
  Wire.setClock(400000);
  Wire.onReceive(receiveEvent);

  pinMode(ESTOP_INT, INPUT_PULLUP);

  BrushServo.attach(SERVO); 

  for (int index = 0; index < pwmPinCount; index++)
  {
    pinMode(pwmPins[index], OUTPUT);
  }

  for (int index = 0; index < digitalPinCount; index++)
  {
    pinMode(digitalPins[index], OUTPUT);
  }

  settingUp = false;
}

void loop() {
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
      processNewData();
    }
    else
    {
      parseInput();
    }
  }
}
