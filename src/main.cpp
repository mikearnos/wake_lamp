#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "clock_ds3231.h"
#include "time_ntp.h"
#include "outlet_control.h"

#include "..\..\..\wifi.h"
const char* ssid = STASSID;
const char* password = STAPSK;

extern bool clockUpdateTime;
extern void clockGetEpoch();

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // OFF

    outletHardwareSetup(); // calls Wire.begin(), check for and setup i2c expander
    clockCheckHardware(); // check for clock hardware

    clockNTPUpdate(0); // update DS3231 if power was lost

    Serial.printf("First run at %s\n", clockGetTimeDateString(0));

    clockSetAlarms(); // enable alarms after we have the correct time
}

void loop()
{
    clockCheckEvents(); // check events once a minute and once a month
}

void handleEventMinutes(void)
{
    Serial.printf("Alarm 2 went off at %s\n", clockGetTimeDateString(0));
    //clockGetTimeDateString();

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
