#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

#include "clock_ds3231.h"
#include "time_ntp.h"
#include "outlet_control.h"

#include "..\..\..\wifi.h"
const char* ssid = STASSID;
const char* password = STAPSK;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // OFF

    outletHardwareSetup(); // calls Wire.begin(), check for and setup i2c expander
    clockCheckHardware(); // check for clock hardware

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.printf("\nConnected to %s", ssid);
    Serial.printf(" - IP address: %s\n", WiFi.localIP().toString().c_str());

    clockNTPUpdate(0); // update DS3231 if power was lost

    Serial.printf("First trigger at %s\n", timeDateString());
    clockHandleEventMinutes();

    clockSetAlarms(); // enable alarms after we have the correct time
}

void loop()
{
    clockHandleEvents(); // check events once a minute and once a month
}

void clockHandleEventMinutes(void)
{
    /*if (webSocket.connectedClients() > 0) {
        char* jsonString;
        jsonString = makeJsonString();
        webSocket.broadcastTXT(jsonString);
        Serial.printf("WS*: Sent current status: %s\n", jsonString);
    }*/
}