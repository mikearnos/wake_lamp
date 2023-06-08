#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "clock_ds3231.h"
#include "time_ntp.h"
#include "outlet_control.h"

#include "..\..\..\wifi.h"
const char* ssid = STASSID;
const char* password = STAPSK;

const int analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0; // value read from the pot

extern bool clockUpdateTime;

extern void oledSetup();
extern void oledGo(char*);
int bufferADC(int);
int movingWindowADC(int);

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // OFF

    outletHardwareSetup(); // calls Wire.begin(), check for and setup i2c expander
    clockCheckHardware(); // check for clock hardware

    clockNTPUpdate(0); // update DS3231 if power was lost

    oledSetup();

    Serial.printf("First run at %s\n", clockGetTimeDateString(0));
    oledGo(clockGetTimeDateString(0));

    clockSetAlarms(); // enable alarms after we have the correct time
}

void loop()
{
    char bufferStr[7]; //the ASCII of the integer will be stored in this char array
    const char* hourStrings[] = { "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
    const char* minStrings[] = { "00", "30" };
    const char* noonStrings[] = { "AM", "PM" };
    //sensorValue = bufferADC(8);
    sensorValue = movingWindowADC(4);
    sensorValue = constrain(sensorValue, 20, 1024);
    sensorValue = map(sensorValue, 20, 1024, 0, 47);

    //itoa(sensorValue, bufferStr, 10); //(integer, yourBuffer, base)

    static uint32_t oledRefresh = millis();

    strcpy(bufferStr, hourStrings[sensorValue / 2]);
    strcat(bufferStr, ":");
    strcat(bufferStr, minStrings[sensorValue % 2]);
    strcat(bufferStr, " ");
    strcat(bufferStr, noonStrings[(sensorValue / 24) % 2]);

    //delay(20);
    if (millis() - oledRefresh > 20) {
        oledRefresh = millis();
        oledGo(bufferStr);
    }

    clockCheckEvents(); // launch events once a minute and once a month
}

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
    buffer[bufferSize - 1] = analogRead(analogInPin); // insert new value at the end

    for (int i = 0; i < bufferSize; i++) { // average the buffer
        average += buffer[i];
    }
    average = average / bufferSize;

    //Serial.printf("raw: %d buffer: %d ", buffer[bufferSize - 1], average);

    return average;
}

int movingWindowADC(int deviation)
{
    static int stableRead;

    int newRead = analogRead(analogInPin);
    if (newRead >= stableRead + deviation || newRead <= stableRead - deviation) {
        stableRead = newRead;
    }

    //Serial.printf("movingWindow: %d\n", stableRead);

    return stableRead;
}

void handleEventMinutes(void)
{
    time_t localTime = clockGetLocalTime();
    Serial.printf("Alarm 2 went off at %s\n", clockGetTimeDateString(localTime));
    oledGo(clockGetTimeDateString(0));

    if (clockUpdateTime) {
        clockNTPUpdate(1); // force an NTP update
    }
}

void connectWifi(void)
{
    WiFi.forceSleepWake();
    delay(1);
    WiFi.mode(WIFI_STA);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.printf("\nConnected to %s", ssid);
    Serial.printf(" - IP address: %s\n", WiFi.localIP().toString().c_str());
}

void disconnectWifi(void)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}
