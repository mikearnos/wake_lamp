#include <DFRobot_DF1201S.h>
#include <SoftwareSerial.h>

#define DFRX D4
#define DFTX D7

#define SND_CAPCOM 1

SoftwareSerial DF1201SSerial(DFRX, DFTX); //RX  TX
DFRobot_DF1201S DF1201S;

int totalFiles;

void dfSetup(void)
{
    DF1201SSerial.begin(9600);

    for (int i = 5; !DF1201S.begin(DF1201SSerial) && i; i--) {
        if (i == 1) {
            Serial.println("DF1201S init failed, please check the wire connection!");
            for (int i = 0; i < 3; i++) {
                //purpleFlash(255);
                delay(200);
                //purpleFlash(0);
                delay(200);
            }
            return;
        }
    }

    // need to power cycle module after these are changed
    //DF1201S.setBaudRate(9600);
    //DF1201A.setLED(off)
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
}

uint16_t dfPlay(int fileID)
{
    DF1201S.playFileNum(fileID);
    uint16_t totalTime = DF1201S.getTotalTime();
    if (totalTime == 0)
        return 1;

    return totalTime;
}
