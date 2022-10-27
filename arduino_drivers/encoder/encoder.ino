#include <util/atomic.h>
#include "src/I2C_PROCESSING.h"

// Encoder input pins (only 1 needs to be interrupt)
#define ENC_FL_A 2
#define ENC_FL_B 4

#define ENC_FR_A 3
#define ENC_FR_B 5

#define ENC_BL_A 18
#define ENC_BL_B 16

#define ENC_BR_A 19
#define ENC_BR_B 17

// Tick count for 1 rev
#define ENC_FL_REV 8900
#define ENC_FR_REV 8900
#define ENC_BL_REV 8900
#define ENC_BR_REV 8900

// Constants
const int measure_interval = 1;       // interval for velocity measurement (s)
const float rev_to_rad = 6.283185307; // 2pi

// Variables
volatile int posFL = 0;
volatile int posFR = 0;
volatile int posBL = 0;
volatile int posBR = 0;
long prevT = 0;
float ang_vel_FL = 0;
float ang_vel_FR = 0;
float ang_vel_BL = 0;
float ang_vel_BR = 0;
char final_FL[8];
char final_FR[8];
char final_BL[8];
char final_BR[8];
char speedChars[31];

void requestEvent()
{
  Wire.write(speedChars, 31);
}

void readFL()
{
  int fl = digitalRead(ENC_FL_B);
  if (fl > 0)
  {
    posFL++;
  }
  else
  {
    posFL--;
  }
}

void readFR()
{
  int fr = digitalRead(ENC_FR_B);
  if (fr > 0)
  {
    posFR++;
  }
  else
  {
    posFR--;
  }
}

void readBL()
{
  int bl = digitalRead(ENC_BL_B);
  if (bl > 0)
  {
    posBL++;
  }
  else
  {
    posBL--;
  }
}

void readBR()
{
  int br = digitalRead(ENC_BR_B);
  if (br > 0)
  {
    posBR++;
  }
  else
  {
    posBR--;
  }
}

void setup()
{
//  Serial.begin(9600);
  Wire.begin(0x65);
  Wire.setClock(400000);
  Wire.onRequest(requestEvent);
  pinMode(ENC_FL_A, INPUT);
  pinMode(ENC_FL_B, INPUT);
  pinMode(ENC_FR_A, INPUT);
  pinMode(ENC_FR_B, INPUT);
  pinMode(ENC_BL_A, INPUT);
  pinMode(ENC_BL_B, INPUT);
  pinMode(ENC_BR_A, INPUT);
  pinMode(ENC_BR_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_FL_A), readFL, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_FR_A), readFR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_BL_A), readBL, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_BR_A), readBR, RISING);
}

void loop()
{

  // time difference
  long currT = micros();
  float deltaT = ((float)(currT - prevT)) / (1.0e6);

  if (deltaT > measure_interval)
  {

    prevT = currT;

    // Read the position in an atomic block to avoid a potential
    // misread if the interrupt coincides with this code running
    // see: https://www.arduino.cc/reference/en/language/variables/variable-scope-qualifiers/volatile/
    int posb_FL = 0;
    int posb_FR = 0;
    int posb_BL = 0;
    int posb_BR = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      posb_FL = posFL;
      posb_FR = posFR;
      posb_BL = posBL;
      posb_BR = posBR;
    }

    // FOR MEASURING ENCODER REV
    //    Serial.print("Front Left: ");
    //    Serial.println(posb_FL);
    //    Serial.print("Front Right: ");
    //    Serial.println(posb_FR);
    //    Serial.print("Back Left: ");
    //    Serial.println(posb_BL);
    //    Serial.print("Back Right: ");
    //    Serial.println(posb_BR);

    // rad/s
    ang_vel_FL = (float)(posb_FL / deltaT / ENC_FL_REV * rev_to_rad);
    ang_vel_FR = (float)(posb_FR / deltaT / ENC_FR_REV * rev_to_rad);
    ang_vel_BL = (float)(posb_BL / deltaT / ENC_BL_REV * rev_to_rad);
    ang_vel_BR = (float)(posb_BR / deltaT / ENC_BR_REV * rev_to_rad);
//    Serial.print("Front Left: ");
//    Serial.println(ang_vel_FL);
//    Serial.print("Front Right: ");
//    Serial.println(ang_vel_FR);
//    Serial.print("Back Left: ");
//    Serial.println(ang_vel_BL);
//    Serial.print("Back Right: ");
//    Serial.println(ang_vel_BR);
//    Serial.println();

    dtostrf(ang_vel_FL, 7, 3, final_FL);
    strcat(speedChars, final_FL);
    strcat(speedChars, ",");
    dtostrf(ang_vel_FR, 7, 3, final_FR);
    strcat(speedChars, final_FR);
    strcat(speedChars, ",");
    dtostrf(ang_vel_BL, 7, 3, final_BL);
    strcat(speedChars, final_BL);
    strcat(speedChars, ",");
    dtostrf(ang_vel_BR, 7, 3, final_BR);
    strcat(speedChars, final_BR);
//    Serial.print("speedChars: ");
//    Serial.println(speedChars);
    
    speedChars[0] = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      posFL = 0;
      posFR = 0;
      posBL = 0;
      posBR = 0;
    }
  }
}
