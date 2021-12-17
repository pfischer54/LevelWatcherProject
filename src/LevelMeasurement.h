#ifndef LEVELMEASUREMENT_H
#define LEVELMEASUREMENT_H

#include "Particle.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>
#include "JsonParserGeneratorRK.h"
#include <vector>

// Two averaging buckets are provided, short and long averaging
const int LONG_SAMPLE_SIZE = 4;                    // number of measurements to average for long term average;
const int SHORT_SAMPLE_SIZE = 3;                    // number of measurements to average for short term average;

class LevelMeasurement
{
public:
LevelMeasurement();
LevelMeasurement(String sid);

bool isZeroingInProgress(void);
void setZeroingInProgress(void);

virtual void measureLevel(void) = 0;

//xxx interfaceType;
//xxx int adcChannel;  //  adc channel number for this sensor

protected:
String sensorId;
bool zeroingInProgress;
int waterLevelSampleReading;

private:
String data = String(80);
};

#endif