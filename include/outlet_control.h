#define OUTLET_NUM 4
#define OUTLET_START P0 // using PCF8574 output pins P0 - P3
#define INPUT_START P4 // using PCF8574 input pins P4 - P7
#define PCF8574_ADDR 0x20

#define PCF8574_INT_PIN D5 // wemos D1 mini pin

#define TOP_BUTTON INPUT_START

#define MODES 3
#define MODE_ALARM_OFF 0
#define MODE_ALARM_SET 1
#define MODE_CLOCK 2

void outletLoop(void);
int outletInitHW(void);
void outletGetStatus(uint8_t*);
void outletOn(int);
void outletOff(int);
