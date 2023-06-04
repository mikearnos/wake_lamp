#include "PCF8574.h" // by Renzo Mischianti
#include "outlet_control.h"

PCF8574 relayBoard(PCF8574_ADDR);
uint8_t outletStatus[OUTLET_NUM] = { 0 };

void outletHardwareSetup()
{
    // Set PCF8574 pinMode to OUTPUT
    // PCF8574 outputs default to HIGH (relays off)
    relayBoard.pinMode(OUTLET_START, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 1, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 2, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 3, OUTPUT);

    relayBoard.begin(); // *** calls Wire.begin() ***

    Wire.beginTransmission(PCF8574_ADDR);
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        Serial.println("PCF8574 IO expander not found");
        return;
    }

    for (int i = 0; i < OUTLET_NUM; i++) {
        outletOn(i);
        delay(200);
        outletOff(i);
    }
}

void outletGetStatus(uint8_t* webData)
{
    for (int i = 0; i < OUTLET_NUM; i++)
        webData[i] = outletStatus[i];
}

void outletOn(int outlet)
{
    if (outlet >= OUTLET_NUM || outlet < 0)
        return;

    relayBoard.digitalWrite(OUTLET_START + outlet, LOW);
    outletStatus[outlet] = 1;
}

void outletOff(int outlet)
{
    if (outlet >= OUTLET_NUM || outlet < 0)
        return;

    relayBoard.digitalWrite(OUTLET_START + outlet, HIGH);
    outletStatus[outlet] = 0;
}
