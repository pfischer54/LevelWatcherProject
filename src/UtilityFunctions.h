#ifndef UTILITYFUNCTION_H
#define UTILITYFUNCTION_H

// Forward declarations
class Adafruit_ADS1115;
class LevelMeasurement;

void sos();
void blink(unsigned long onTime);
void blinkShort(int times);
void blinkVeryShort(int times);
void blinkLong(int times);
void initalizeAdc( Adafruit_ADS1115 ads);
uint8_t getSensorIndex(String sensorId);
bool isAnyZeroingInProgress(LevelMeasurement * lm[]);
int parseValue(char* data);
#endif