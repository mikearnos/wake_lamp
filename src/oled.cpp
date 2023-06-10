#include <U8g2lib.h>
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
/*Note: In v2.29 the returned value might be (a little bit) larger than in
  the previous versions of u8g2. Undef U8G2_BALANCED_STR_WIDTH_CALCULATION
  in u8g2.h to restore the old behaviour.*/

void oledSetup(void)
{
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB24_tr);
}

void oledGo(char* string, int value, int horizontal)
{
    int strWidth;
    u8g2.clearBuffer();
    //u8g2.drawFrame(3,7,25,15);
    u8g2.setFont(u8g2_font_ncenB24_tr);
    if (value == 0) {
        u8g2.drawButtonUTF8(62, 30, U8G2_BTN_HCENTER | U8G2_BTN_BW2, 0, 3, 3, string);
    }
    if (value == 1) {
        u8g2.drawButtonUTF8(62, 30, U8G2_BTN_SHADOW1 | U8G2_BTN_INV | U8G2_BTN_HCENTER | U8G2_BTN_BW2, 34, 2, 2, string);
    }

    u8g2.setFont(u8g2_font_ncenB10_tr);

    if (horizontal < 256) {
        horizontal = constrain(horizontal, 0, 255);
        horizontal = map(horizontal, 0, 255, 0, 127 - u8g2.getStrWidth("AM"));
        u8g2.drawStr(horizontal, 63, "AM"); // 24 pixels wide
    } else {
        horizontal -= 256;
        horizontal = constrain(horizontal, 0, 255);
        horizontal = map(horizontal, 0, 255, 0, 127 - u8g2.getStrWidth("PM"));
        u8g2.drawStr(horizontal, 63, "PM"); // 23 pixels wide
    }

    //u8g2.drawLine(horizontal, 61, horizontal, 63);
    //u8g2.drawStr(0, 20, string);
    u8g2.sendBuffer();
    //Serial.printf("%d %d\n", u8g2.getStrWidth("AM"), u8g2.getStrWidth("PM"));
}