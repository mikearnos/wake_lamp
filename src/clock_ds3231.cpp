#include "Arduino.h"
#include <Wire.h>
#include <DS3231.h> // by Andrew Wickert
//#include <time.h>

#include "clock_ds3231.h"
#include "time_ntp.h"

extern void handleEventMinutes(void); // in main.cpp
extern void connectWifi(void); // in main.cpp
extern void disconnectWifi(void); // in main.cpp

DS3231 ds_clock;
RTClib myRTC; // get now()

uint8_t ds_error = 0;
int16_t clockDelayUpdate = 0; // prevents multiple NTP requests in a short time
bool clockUpdateTime = 0;

volatile bool isrTriggered = 0;

IRAM_ATTR void ds3231_interrupt()
{
    isrTriggered = 1;
}

void clockCheckHardware()
{
    Wire.beginTransmission(DS3231_ADDR);
    uint8_t wire_error = Wire.endTransmission();

    if (wire_error != 0) {
        Serial.println("DS3231 not found");
        ds_error = DS3231_NO_DETECT;
        return;
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
        return;
    }
}

void clockSetAlarms()
{
    // set DS3231 alarm to trigger every minute
    ds_clock.setA1Time(ALARM1_DATE, ALARM1_HOUR, ALARM1_MINUTE, 0, ALARM1_BITS, 0, 0, 0);
    ds_clock.setA2Time(0, 0, 0, ALARM2_BITS, 0, 0, 0);
    // enable alarm interrupts and alarm 2 interrupt
    ds_clock.turnOnAlarm(1);
    ds_clock.turnOnAlarm(2);
}

void clockCheckEvents()
{
    if (!isrTriggered || ds_error & DS3231_NO_DETECT)
        return;

    isrTriggered = 0;

    if (ds_clock.checkIfAlarm(1)) { // *months*, clear alarm flag in the DS3231
        Serial.printf("Alarm 1 went off at %s\n", clockGetTimeDateString(0));
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
        Serial.printf("Time updated: %s\n", clockGetTimeDateString(0));

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

char* clockGetTimeDateString(time_t UNIXTime)
{
    static char buffer[25];

    if (!UNIXTime) {
        UNIXTime = clockGetLocalTime();
    }

    struct tm tmnow;
    gmtime_r(&UNIXTime, &tmnow);

    sprintf(buffer, "%d/%d/%d %d:%02d:%02d", tmnow.tm_mon + 1, tmnow.tm_mday,
        tmnow.tm_year - 100, tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);

    return buffer;
}

time_t clockGetLocalTime()
{
    DateTime now = myRTC.now();
    time_t UNIXTime = now.unixtime();

    UNIXTime += TZ_OFFSET;

    if (isDST(UNIXTime)) {
        UNIXTime += 3600; // add 1 hour for DST
    }

    return UNIXTime;
}
