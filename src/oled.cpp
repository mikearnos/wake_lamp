#include <U8g2lib.h>
#include "oled.h"
#include "analog.h"
#include "clock_ds3231.h"

int mode = 2;

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
/*Note: In v2.29 the returned value might be (a little bit) larger than in
  the previous versions of u8g2. Undef U8G2_BALANCED_STR_WIDTH_CALCULATION
  in u8g2.h to restore the old behaviour.*/

void oledSetup(void)
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 15);
    u8g2.print("Booting...");
    u8g2.sendBuffer();
}

void oledLoop(void)
{
    static uint32_t oledRefresh = millis();

    if (millis() - oledRefresh > 10) {
        oledRefresh = millis();

        u8g2.clearBuffer();

        if (mode < 2) {
            //int sensorValue = bufferADC(8); // this routine lags a bit
            int sensorValue = movingWindowADC(3);
            sensorValue = constrain(sensorValue, DEADZONE_LOW, DEADZONE_HIGH); // trim the dead zone

            oledDrawTimeSet(sensorValue);
        } else {
            clockGetTimeDateString(0);
            if (millis() % 2000 >= 1000) { // toggle once a second
                for (int i = 0; clockTimeString[i] != '\0'; i++) {
                    if (clockTimeString[i] == ':') {
                        clockTimeString[i] = ' ';
                    } else
                        continue;
                }
            }
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.drawButtonUTF8(62, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW2, 0, 3, 3, clockTimeString);
        }

        if (mode < 2) {
            checkerPattern(u8g2.getBufferPtr(), 1);
        }

        u8g2.sendBuffer();
    }
}

void checkerPattern(uint8_t* buffer, uint8_t pixelSize)
{
    static bool flipFlop;

    if (millis() % 2000 >= 1000) { // toggle once a second
        flipFlop = 1;
    } else {
        flipFlop = 0;
    }

    if (pixelSize == 1) { // 1x1 checker pattern
        const uint8_t mesh[] = {
            0xAA, 0x55, // drawn when flipFlop == 0
            0x55, 0xAA // drawn when flipFlop == 1
        };
        for (int i = 128 * (4 / pixelSize); i; i--) {
            *buffer++ &= mesh[0 + (flipFlop * 2)];
            *buffer++ &= mesh[1 + (flipFlop * 2)];
        }
    }
    if (pixelSize == 2) { // 2x2 checker pattern
        const uint8_t mesh[] = {
            0xCC, 0x33, // drawn twice when flipFlop == 0
            0x33, 0xCC // drawn twice when flipFlop == 1
        };
        for (int i = 128 * (4 / pixelSize); i; i--) {
            for (int j = 2; j; j--) {
                *buffer++ &= mesh[0 + (flipFlop * 2)];
            }
            for (int j = 2; j; j--) {
                *buffer++ &= mesh[1 + (flipFlop * 2)];
            }
        }
    }
}

void oledBootPrint(const char* string)
{
    static int vPos;
    if (vPos == 0) {
        u8g2.clearBuffer();
    }

    vPos += 15;
    //u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, vPos);
    u8g2.print(string);
    u8g2.sendBuffer();
}

void oledDrawTimeSet(int sensorValue)
{
    char bufferStr[6]; //the ASCII of the time will be stored in this char array "12:00\n"
    const char* hourStrings[] = { "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
    const char* minStrings[] = { "00", "30" };

    int horizontalValue = map(sensorValue, DEADZONE_LOW, DEADZONE_HIGH, 0, 511);
    int halfHours = map(sensorValue, DEADZONE_LOW, DEADZONE_HIGH, 0, 47); // amount of 30 minute slices in the day

    strcpy(bufferStr, hourStrings[halfHours / 2]);
    strcat(bufferStr, ":");
    strcat(bufferStr, minStrings[halfHours % 2]);

    //u8g2.clearBuffer();

    // draw the time with a box around it
    u8g2.setFont(u8g2_font_ncenB24_tr);
    int value = (halfHours / 24) % 2;
    if (value == 0) { // AM
        u8g2.drawButtonUTF8(62, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW2, 0, 3, 3, bufferStr);
    }
    if (value == 1) { // PM
        u8g2.drawButtonUTF8(62, 30, U8G2_BTN_SHADOW1 | U8G2_BTN_INV | U8G2_BTN_HCENTER | U8G2_BTN_BW2, 34, 2, 2, bufferStr);
    }

    u8g2.setFont(u8g2_font_ncenB10_tr);

    if (horizontalValue < 256) {
        horizontalValue = constrain(horizontalValue, 0, 255);
        horizontalValue = map(horizontalValue, 0, 255, 0, 127 - u8g2.getStrWidth("AM"));
        u8g2.drawStr(horizontalValue, 63, "AM"); // 24 pixels wide
    } else {
        horizontalValue -= 256;
        horizontalValue = constrain(horizontalValue, 0, 255);
        horizontalValue = map(horizontalValue, 0, 255, 0, 127 - u8g2.getStrWidth("PM"));
        u8g2.drawStr(horizontalValue, 63, "PM"); // 23 pixels wide
    }

    //u8g2.sendBuffer();
    //Serial.printf("%d %d\n", u8g2.getStrWidth("AM"), u8g2.getStrWidth("PM"));
}