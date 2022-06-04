#include "SERIAL_PROCESSING.h"

using namespace LibSerialConstants;

char receivedChars[numChars];
int payload[numInputs];
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

void parseInput()
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
}