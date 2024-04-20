#include "Arduino.h"
#include "analog.h"

int bufferADC(int bufferSize)
{
#define MAX_BUFFER_SIZE 16
    static int buffer[MAX_BUFFER_SIZE];
    int average = 0;

    if (bufferSize > MAX_BUFFER_SIZE) {
        return 0;
    }
    for (int i = 0; i < bufferSize - 1; i++) { // shift values before inserting new value
        buffer[i] = buffer[i + 1];
    }
    buffer[bufferSize - 1] = analogRead(ANALOG_IN_PIN); // insert new value at the end

    for (int i = 0; i < bufferSize; i++) { // average the buffer
        average += buffer[i];
    }
    average = average / bufferSize;

    //Serial.printf("raw: %d buffer: %d \n", buffer[bufferSize - 1], average);

    return average;
}

int movingWindowADC(int deviation)
{
    static int stableRead;

    int newRead = analogRead(ANALOG_IN_PIN);
    if (newRead > stableRead + deviation || newRead < stableRead - deviation) {
        stableRead = newRead;
    }

    //Serial.printf("raw: %d    movingWindow: %d\n", newRead, stableRead);

    return stableRead;
}
