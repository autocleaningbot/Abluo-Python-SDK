#include "src/Encoder.h"
#include <Wire.h>

#define ENC_FL_A 19
#define ENC_FL_B 8

#define ENC_FR_A 18
#define ENC_FR_B 9

#define ENC_BL_A 2
#define ENC_BL_B 10

#define ENC_BR_A 3
#define ENC_BR_B 11

// Tick count for 1 rev
#define ENC_FL_REV 550
#define ENC_FR_REV 550
#define ENC_BL_REV 550
#define ENC_BR_REV 550

// Digital Pins Encoders
const byte digitalEncoderPinCount = 8;
const byte digitalEncoderPins[digitalEncoderPinCount] = {ENC_FL_A, ENC_FL_B, ENC_FR_A, ENC_FR_B, ENC_BL_A, ENC_BL_B, ENC_BR_A, ENC_BR_B};

//Declare encoder function
Encoder Enc_FL(ENC_FL_A, ENC_FL_B);
Encoder Enc_FR(ENC_FR_A, ENC_FR_B);
Encoder Enc_BL(ENC_BL_A, ENC_BL_B);
Encoder Enc_BR(ENC_BR_A, ENC_BR_B);

//Time variables
long previousMillis = 0;
long currentMillis = 0;

// Constants
const int measure_freq = 100;       // freq for velocity measurement (hz)
const float twopi = 6.283185307; // 2pi

//Encoder variables
volatile long currFL;
volatile long currFR;
volatile long currBL;
volatile long currBR;
volatile long prevFL = 0;
volatile long prevFR = 0;
volatile long prevBL = 0;
volatile long prevBR = 0;
float ang_vel_FL = 0;
float ang_vel_FR = 0;
float ang_vel_BL = 0;
float ang_vel_BR = 0;
char padded_FL[8];
char padded_FR[8];
char padded_BL[8];
char padded_BR[8];
char speedChars[32];
long prevT = 0;

void requestEvent()
{
  currFL = Enc_FL.read();
  currFR = Enc_FR.read();
  currBL = Enc_BL.read();
  currBR = Enc_BR.read();
  long currT = millis();
  float deltaT = (float)(currT - prevT);

  ang_vel_FL = (float)((currFL - prevFL) * twopi / deltaT * 1000 / ENC_FL_REV);
  ang_vel_FR = (float)((currFL - prevFR) * twopi / deltaT * 1000 / ENC_FR_REV);
  ang_vel_BL = (float)((currBL - prevBL) * twopi / deltaT * 1000 / ENC_BL_REV);
  ang_vel_BR = (float)((currBR - prevBR) * twopi / deltaT * 1000 / ENC_BR_REV);

  speedChars[0] = 0;
  dtostrf(ang_vel_FL, 7, 3, padded_FL);
  strcat(speedChars, padded_FL);
  strcat(speedChars, ",");
  dtostrf(ang_vel_FR, 7, 3, padded_FR);
  strcat(speedChars, padded_FR);
  strcat(speedChars, ",");
  dtostrf(ang_vel_BL, 7, 3, padded_BL);
  strcat(speedChars, padded_BL);
  strcat(speedChars, ",");
  dtostrf(ang_vel_BR, 7, 3, padded_BR);
  strcat(speedChars, padded_BR);

  Wire.write(speedChars, 31);

  prevFL = currFL;
  prevFR = currFR;
  prevBL = currBL;
  prevBR = currBR;
  prevT = millis();
}

void setup() 
{
  Wire.begin(0x65);
  Wire.setClock(400000);
  Wire.onRequest(requestEvent);

  // Setup Encoder Pins
  for (int index = 0; index < digitalEncoderPinCount; index++)
  {
    pinMode(digitalEncoderPins[index], INPUT_PULLUP);
  }
}

void loop() {

}
