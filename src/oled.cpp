#include <U8g2lib.h>
#include "analog.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
/*Note: In v2.29 the returned value might be (a little bit) larger than in
  the previous versions of u8g2. Undef U8G2_BALANCED_STR_WIDTH_CALCULATION
  in u8g2.h to restore the old behaviour.*/

void oledSetup(void)
{
    u8g2.begin();
    //u8g2.setFont(u8g2_font_ncenB24_tr);
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, 15);
    u8g2.print("Booting...");
    u8g2.sendBuffer();
}

void oledBootPrint(const char* string)
{
    static int vPos = 0;
    if (vPos == 0) {
        u8g2.clearBuffer();
    }

    vPos += 15;
    //u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, vPos);
    u8g2.print(string);
    u8g2.sendBuffer();
}

void oledGo(int sensorValue, bool systemEnabled)
{
    char bufferStr[6]; //the ASCII of the time will be stored in this char array "12:00\n"
    const char* hourStrings[] = { "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11" };
    const char* minStrings[] = { "00", "30" };

    int horizontalValue = map(sensorValue, DEADZONE_LOW, DEADZONE_HIGH, 0, 511);
    int halfHours = map(sensorValue, DEADZONE_LOW, DEADZONE_HIGH, 0, 47); // amount of 30 minute slices in the day

    strcpy(bufferStr, hourStrings[halfHours / 2]);
    strcat(bufferStr, ":");
    strcat(bufferStr, minStrings[halfHours % 2]);

    u8g2.clearBuffer();

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

    if (!systemEnabled) {
        uint8_t* test = u8g2.getBufferPtr();
        for (int i = 128 * 2; i; i--) {
            for (int j = 2; j; j--) {
                *test++ &= 0xCC;
            }
            for (int j = 2; j; j--) {
                *test++ &= 0x33;
            }
        }
    }

    u8g2.sendBuffer();
    //Serial.printf("%d %d\n", u8g2.getStrWidth("AM"), u8g2.getStrWidth("PM"));
}