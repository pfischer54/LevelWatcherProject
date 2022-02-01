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

String sensorId = {"Unkown"};
bool isZeroingInProgress(void);
void setZeroingInProgress(void);
virtual void measureLevel(void) = 0;

protected:
void publishLevel(int);

int sample;
bool zeroingInProgress;
int waterLevelSampleReading;
String data = String(80);

};

#endif