#include "Arduino.h"
#include <Wire.h>
#include <DS3231.h> // by Andrew Wickert
//#include <time.h>

#include "clock_ds3231.h"
#include "time_ntp.h"
#include "oled.h"

extern void handleEventMinutes(void); // in main.cpp
extern void connectWifi(void); // in main.cpp
extern void disconnectWifi(void); // in main.cpp

DS3231 ds_clock;
RTClib myRTC; // get now()

char* clockTimeDateString;
char* clockTimeString;

uint8_t ds_error = 0;
int16_t clockDelayUpdate = 0; // prevents multiple NTP requests in a short time
bool clockUpdateTime = 0;

volatile bool isrTriggered = 0;

IRAM_ATTR void ds3231_interrupt()
{
    isrTriggered = 1;
}

int clockInitHW()
{
    Wire.beginTransmission(DS3231_ADDR);
    uint8_t wire_error = Wire.endTransmission();

    if (wire_error != 0) {
        Serial.println("DS3231 not found");
        oledBootPrint("Clock FAIL!");
        ds_error = DS3231_NO_DETECT;
        return 1;
    }

    if (!ds_clock.oscillatorCheck()) {
        //oscillator has been off for some reason, the time is probably not correct.
        ds_error = DS3231_LOST_POWER;
    }

    // set esp8266 pin interupt to detect DS3231 alarms
    pinMode(DS3231_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(DS3231_INT_PIN), ds3231_interrupt, FALLING);

    // -=-=-=-=-=-=-=-=-=-=-
    // check EEPROM hardware
    // -=-=-=-=-=-=-=-=-=-=-
    Wire.beginTransmission(AT24C32_ADDR);
    wire_error = Wire.endTransmission();

    if (wire_error != 0) {
        Serial.println("AT24C32_ADDR EEPROM not found");
    }

    oledBootPrint("Clock OK!");
    delay(1000);

    return 0;
}

void clockSetAlarms()
{
    // alarms seem to not work sometimes after losing power but with a battery installed without this
    ds_clock.turnOffAlarm(1);
    ds_clock.turnOffAlarm(2);
    // set DS3231 alarm to trigger every minute
    ds_clock.setA1Time(ALARM1_DATE, ALARM1_HOUR, ALARM1_MINUTE, 0, ALARM1_BITS, 0, 0, 0);
    ds_clock.setA2Time(0, 0, 0, ALARM2_BITS, 0, 0, 0);
    // enable alarm interrupts and alarm 2 interrupt
    ds_clock.turnOnAlarm(1);
    ds_clock.turnOnAlarm(2);
}

void clockLoop()
{
    if (!isrTriggered || ds_error & DS3231_NO_DETECT)
        return;

    isrTriggered = 0;

    clockGetTimeDateString(0); // read DS3231 and create strings containing time

    if (ds_clock.checkIfAlarm(1)) { // *months*, clear alarm flag in the DS3231
        Serial.printf("Alarm 1 went off at %s\n", clockTimeDateString);
        clockUpdateTime = 1;
    }
    if (ds_clock.checkIfAlarm(2)) { // *minutes*, clear alarm flag in the DS3231
        handleEventMinutes();

        if (clockDelayUpdate > 0) // only allow an NTP update once per 2 hours
            clockDelayUpdate--; // counts down from 120 every minute
    }
}

void clockNTPUpdate(int16_t force)
{
    if (ds_error & DS3231_NO_DETECT) {
        return;
    }
    if (clockDelayUpdate > 0) {
        Serial.printf("INFO: NTP request sooner than %d hours\n", NTP_DELAY_HOURS);
        return;
    }
    if (!ds_error && !force) { // hardware is OK and not forced
        return;
    }
    if (ds_error & DS3231_LOST_POWER) {
        // flag cleared on NTP update below
        Serial.println("ERROR: DS3231 Lost Power");
    }

    oledBootPrint("NTP update...");
    Serial.println("Updating time via NTP.");
    connectWifi();
    startUDP();

    uint32_t time = 0;
    int16_t loop = 20;
    while (!time && loop) { // wait for UDP response
        time = getTime();
        loop--;
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    stopUDP();
    disconnectWifi();

    if (loop) { // if UDP didn't timeout
        clockSetEpoch(time); // also resets the Oscillator Stop Flag
        clockGetTimeDateString(0);
        Serial.printf("Time updated: %s\n", clockTimeDateString);

        ds_error &= ~DS3231_LOST_POWER; // clear the flag
        clockDelayUpdate = NTP_DELAY_MINS; // delay next NTP update
        clockUpdateTime = 0;
    } else {
        Serial.println("ERROR: Couldn't get time from NTP server, will retry.");
        clockUpdateTime = 1;
    }
}

void clockSetEpoch(time_t epoch)
{
    // setEpoch function gives the epoch as parameter and feeds the RTC
    // epoch = UnixTime and starts at 01.01.1970 00:00:00
    struct tm tmnow;
    gmtime_r(&epoch, &tmnow);

    ds_clock.setSecond(tmnow.tm_sec); /* seconds */ // This function also resets the Oscillator Stop Flag
    ds_clock.setMinute(tmnow.tm_min); /* minutes */
    ds_clock.setHour(tmnow.tm_hour); /* hours */
    ds_clock.setDoW(tmnow.tm_wday + 1); /* day of the week */
    ds_clock.setDate(tmnow.tm_mday); /* day of the month */
    ds_clock.setMonth(tmnow.tm_mon + 1); /* month */
    ds_clock.setYear(tmnow.tm_year - 100); /* year */
}

void clockGetTimeDateString(time_t UNIXTime)
{
    static char timeDateBuffer[25];
    static char timeBuffer[25];

    const char* meridiemString[] = { "AM", "PM" };
    int meridiem = 0;

    if (!UNIXTime) {
        UNIXTime = clockGetLocalTime(); // reads from the DS3231
    }

    struct tm tmnow;
    gmtime_r(&UNIXTime, &tmnow);

    sprintf(timeDateBuffer, "%d/%d/%d %d:%02d:%02d", tmnow.tm_mon + 1, tmnow.tm_mday,
        tmnow.tm_year - 100, tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);

    int hour = tmnow.tm_hour;

    if (hour > 11) {
        meridiem = 1;
    }
    if (hour > 12) {
        hour -= 12;
    }
    if (hour == 0) // midnight
        hour = 12;

    sprintf(timeBuffer, "%d:%02d %s", hour, tmnow.tm_min, meridiemString[meridiem]);

    clockTimeDateString = timeDateBuffer;
    clockTimeString = timeBuffer;
}

time_t clockGetLocalTime()
{
    DateTime now = myRTC.now();
    time_t UNIXTime = now.unixtime();

    UNIXTime += TZ_OFFSET * 3600;

    if (isDST(UNIXTime)) {
        UNIXTime += 3600; // add 1 hour for DST
    }

    return UNIXTime;
}
