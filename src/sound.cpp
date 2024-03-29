#include <DFRobot_DF1201S.h>
#include <SoftwareSerial.h>
#include "oled.h"

#define DFRX D4
#define DFTX D7

#define SND_CAPCOM 1

SoftwareSerial DF1201SSerial(DFRX, DFTX); //RX  TX
DFRobot_DF1201S DF1201S;

int totalFiles;

int soundInitHW(void)
{
    DF1201SSerial.begin(115200);

    for (int i = 5; !DF1201S.begin(DF1201SSerial) && i; i--) {
        if (i == 1) {
            Serial.println("DF1201S init failed, check the wire connection!");
            oledBootPrint("Sound FAIL!");
            return 1;
        }
        delay(200);
    }

    // need to power cycle module after these are changed
    //DF1201S.setBaudRate(9600);
    //DF1201S.setLED(false);
    //DF1201S.setPrompt(false);

    DF1201S.setVol(25);
    Serial.print("VOL: ");
    Serial.println(DF1201S.getVol());

    DF1201S.switchFunction(DF1201S.MUSIC);
    DF1201S.setPlayMode(DF1201S.SINGLE);

    //Serial.print("PlayMode: ");
    //Serial.println(DF1201S.getPlayMode());

    totalFiles = DF1201S.getTotalFile();
    Serial.print("TotalFiles: ");
    Serial.println(totalFiles);

    oledBootPrint("Sound OK!");
    delay(1000);

    return 0;
}

uint16_t soundPlay(int fileID)
{
    DF1201S.playFileNum(fileID);
    uint16_t totalTime = DF1201S.getTotalTime();
    if (totalTime == 0)
        return 1;

    return totalTime;
}
