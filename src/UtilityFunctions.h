#ifndef UTILITYFUNCTION_H
#define UTILITYFUCTIONS_H

#include <Adafruit_ADS1015.h>

void sos();
void blink(unsigned long onTime);
void blinkShort(int times);
void blinkLong(int times);
void initalizeAdc( Adafruit_ADS1115 ads);
uint8_t getSensorIndex(String sensorId);
bool isAnyZeroingInProgress(LevelMeasurement * lm[]);
int parseValue(char* data);
#endif