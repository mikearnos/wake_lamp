#define OUTLET_NUM    4
#define OUTLET_START P0 // using PCF8574 output pins P0 - P3
#define PCF8574_ADDR  0x20

void outletHardwareSetup(void);
void outletGetStatus(uint8_t*);
void outletOn(int);
void outletOff(int);
