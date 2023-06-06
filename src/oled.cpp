#include <U8g2lib.h>
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void oledSetup(void)
{
    u8g2.begin();
}

void oledGo(char* string)
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0, 20, string);
    u8g2.sendBuffer();
}