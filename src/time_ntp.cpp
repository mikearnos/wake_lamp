// most code from https://tttapa.github.io/ESP8266/Chap15%20-%20NTP.html

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "time_ntp.h"
#include "clock_ds3231.h"

WiFiUDP UDP; // Create an instance of the WiFiUDP class to send and receive

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* NTPServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
uint8_t NTPBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

unsigned long intervalNTP = 60000; // Request NTP time every minute
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;

unsigned long prevActualTime = 0;

void startUDP()
{
    UDP.begin(123); // Start listening for UDP messages on port 123
    //Serial.print("UDP Local port:\t");
    //Serial.println(UDP.localPort());
    //Serial.println();

    // Resolve the given hostname to an IP address
    if (!WiFi.hostByName(NTPServerName, timeServerIP)) {
        Serial.println("DNS lookup failed.");
        return;
    }
    Serial.print("Time server IP:\t");
    Serial.println(timeServerIP);

    Serial.print("Sending NTP request");
    sendNTPpacket(timeServerIP);
}

void stopUDP()
{
    UDP.stop();
}

void sendNTPpacket(IPAddress& address)
{
    memset(NTPBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
    // Initialize values needed to form NTP request
    NTPBuffer[0] = 0b11100011; // LI, Version, Mode
    // send a packet requesting a timestamp:
    UDP.beginPacket(address, 123); // NTP requests are to port 123
    UDP.write(NTPBuffer, NTP_PACKET_SIZE);
    UDP.endPacket();
}

void udpLoop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - prevNTP > intervalNTP) {
        // If a minute has passed since last NTP request
        prevNTP = currentMillis;
        Serial.println("\r\nSending NTP request ...");
        sendNTPpacket(timeServerIP); // Send an NTP request
    }

    uint32_t time = getTime(); // Check if an NTP response has arrived and get the (UNIX) time

    if (time) { // If a new timestamp has been received
        timeUNIX = time;
        clockSetEpoch(time);
        Serial.print("NTP response:\t");
        Serial.println(timeUNIX);
        lastNTPResponse = currentMillis;
    }

    uint32_t actualTime = timeUNIX + (currentMillis - lastNTPResponse) / 1000;
    if (actualTime != prevActualTime && timeUNIX != 0) {
        // If a second has passed since last print
        prevActualTime = actualTime;
        Serial.printf("\rUTC time:\t%d:%d:%d   ", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
    }
}

uint32_t getTime()
{
    if (UDP.parsePacket() == 0) { // If there's no response (yet)
        return 0;
    }
    UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // Combine the 4 timestamp bytes into one 32-bit number
    uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];

    // Convert NTP time to a UNIX timestamp:
    // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
    const uint32_t seventyYears = 2208988800UL;

    // subtract seventy years:
    uint32_t UNIXTime = NTPTime - seventyYears;

    return UNIXTime;
}

inline int getSeconds(uint32_t UNIXTime)
{
    return UNIXTime % 60;
}

inline int getMinutes(uint32_t UNIXTime)
{
    return UNIXTime / 60 % 60;
}

inline int getHours(uint32_t UNIXTime)
{
    return UNIXTime / 3600 % 24;
}
