#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "clock_ds3231.h"
#include "time_ntp.h"
#include "outlet_control.h"
#include "analog.h"
#include "oled.h"

#include "..\..\..\wifi.h"
const char* ssid = STASSID;
const char* password = STAPSK;

bool systemEnabled = 1;

extern bool clockUpdateTime;

extern int soundInitHW(void);
extern uint16_t soundPlay(int);

void setup()
{
    int errorHW = 0;
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // OFF

    oledSetup();
    oledBootPrint("booting...");

    errorHW += soundInitHW();
    soundPlay(1); // play first mp3 as startup sound

    errorHW += outletInitHW(); // calls Wire.begin(), check for and setup i2c expander

    errorHW += clockInitHW(); // check for clock hardware

    if (errorHW){
        while(1);
    }

    //clockNTPUpdate(0); // update DS3231 if power was lost

    Serial.printf("First run at %s\n", clockGetTimeDateString(0));
    //oledGo(clockGetTimeDateString(0));

    clockSetAlarms(); // enable alarms after we have the correct time
}

void loop()
{
    static uint32_t oledRefresh = millis();
    if (millis() - oledRefresh > 10) {
        oledRefresh = millis();

        //int sensorValue = bufferADC(8); // this routine lags a bit
        int sensorValue = movingWindowADC(4);
        sensorValue = constrain(sensorValue, DEADZONE_LOW, DEADZONE_HIGH); // trim the dead zone

        oledGo(sensorValue, systemEnabled);
    }

    outletLoop(); // check for input interrupt

    clockLoop(); // launch events once a minute and once a month
}

void handleEventMinutes(void)
{
    time_t localTime = clockGetLocalTime();
    Serial.printf("Alarm 2 went off at %s\n", clockGetTimeDateString(localTime));
    //oledGo(clockGetTimeDateString(0));

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
