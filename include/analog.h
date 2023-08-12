#define ANALOG_IN_PIN A0 // ESP8266 Analog Pin ADC0 = A0

#define DEADZONE_LOW 20
#define DEADZONE_HIGH 1024

int bufferADC(int);
int movingWindowADC(int);
