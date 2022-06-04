#ifndef SERIAL_PROCESSING_H
#define SERIAL_PROCESSING_H
#include <Arduino.h>

namespace LibSerialConstants
{
    const byte numChars = 32;
    const byte numInputs = 4;
}

extern boolean newData;
extern int payload[];
extern char receivedChars[];

static char *input;
static char *token;
extern void recvWithEndMarker();
extern void parseInput();

#endif