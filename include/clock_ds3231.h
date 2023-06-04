#define DS3231_ADDR 0x68
#define AT24C32_ADDR 0x57

#define DS3231_INT_PIN D6 // wemos D1 mini pin

// self defined error values
#define DS3231_OK         0b00000000
#define DS3231_NO_DETECT  0b00000001
#define DS3231_LOST_POWER 0b00000010

// Set timer delay values
#define ALARM1_DATE   1        // first day of the month
#define ALARM1_HOUR   0        // midnight
#define ALARM1_MINUTE 0
#define ALARM1_BITS 0b0000     // "Alarm when date, hours, minutes, and seconds match"
#define ALARM2_BITS 0b111 << 4 // "Alarm once per minute (00 seconds of every minute)"

void ds3231_interrupt(void);
void clockHandleEvents(void);

void clockCheckHardware(void);
void clockSetAlarms(void);
void clockHandleEventMinutes(void);
void clockNTPUpdate(int16_t);
void clockSetEpoch(time_t);
char * clockTimeDateString(void);
