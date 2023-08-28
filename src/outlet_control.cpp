#include "PCF8574.h" // by Renzo Mischianti
#include "outlet_control.h"
#include "oled.h"

PCF8574 relayBoard(PCF8574_ADDR);
uint8_t outletStatus[OUTLET_NUM] = { 0 };

volatile bool pcfIsrTriggered = 0;

IRAM_ATTR void pcf8574_interrupt()
{
    pcfIsrTriggered = 1;
}

void outletLoop()
{
    static uint32_t debounceDelay;

    if (pcfIsrTriggered) {
        pcfIsrTriggered = 0;
        uint8_t input = relayBoard.digitalRead(TOP_BUTTON, 0);

        if (millis() - debounceDelay > 50) {
            debounceDelay = millis();
            if (!input) {
                if (++mode >= MODES)
                    mode = 0;
                Serial.printf("Mode = %d\n", mode);
            }
        }
    }
}

int outletInitHW()
{
    pcfIsrTriggered = 0;

    // Set PCF8574 pinMode to OUTPUT
    // PCF8574 outputs default to HIGH (relays off)
    relayBoard.pinMode(OUTLET_START, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 1, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 2, OUTPUT);
    relayBoard.pinMode(OUTLET_START + 3, OUTPUT);

    relayBoard.pinMode(INPUT_START, INPUT);

    relayBoard.begin(); // *** calls Wire.begin() ***

    Wire.beginTransmission(PCF8574_ADDR);
    uint8_t error = Wire.endTransmission();

    if (error != 0) {
        Serial.println("PCF8574 IO expander not found");
        oledBootPrint("I/O exp FAIL!");
        return 1;
    }

    // set esp8266 pin interupt
    pinMode(PCF8574_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PCF8574_INT_PIN), pcf8574_interrupt, FALLING);

    for (int i = 0; i < OUTLET_NUM; i++) {
        outletOn(i);
        delay(200);
        outletOff(i);
    }

    oledBootPrint("I/O exp OK!");
    delay(1000);

    return 0;
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
