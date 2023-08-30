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

extern int soundInitHW(void);
extern uint16_t soundPlay(int);

void setup()
{
    int errorHW = 0;
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // OFF

    oledSetup();

    //errorHW += soundInitHW();
    //soundPlay(1); // play first mp3 as startup sound

    errorHW += outletInitHW(); // calls Wire.begin(), check for and setup i2c expander

    errorHW += clockInitHW(); // check for clock hardware

    if (errorHW) {
        while (1)
            ;
    }

    oledBootPrint("NTP update...");
    clockNTPUpdate(0); // update DS3231 if power was lost

    clockGetTimeDateString(0);

    Serial.printf("First run at %s\n", clockTimeDateString);
    //clockTimeString = clockGetTimeString(0);

    clockSetAlarms(); // enable alarms after we have the correct time
}

void loop()
{
    outletLoop(); // check for input interrupt

    clockLoop(); // launch events once a minute and once a month

    oledLoop(); // update the OLED display
}

void handleEventMinutes(void)
{
    time_t localTime = clockGetLocalTime();
    clockGetTimeDateString(localTime);
    Serial.printf("Alarm 2 went off at %s\n", clockTimeDateString);

    Serial.printf("clockTimeString = %s", clockTimeString);
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
