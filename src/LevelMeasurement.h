#ifndef LEVELMEASUREMENT_H
#define LEVELMEASUREMENT_H

//#include "Particle.h"
//#include <RunningAverage.h>
//#include <CellularHelper.h>
#include "Adafruit_ADS1015.h"
//#include "JsonParserGeneratorRK.h"
//#include <vector>

// Two averaging buckets are provided, short and long averaging
const int LONG_SAMPLE_SIZE = 4;                    // number of measurements to average for long term average;
const int SHORT_SAMPLE_SIZE = 3;                    // number of measurements to average for short term average;

class LevelMeasurement
{
public:
LevelMeasurement();
LevelMeasurement(String sid);
LevelMeasurement(String sid, boolean diff, uint sink);

const  int INNER_LOOP_DELAY_COUNT_DEFAULT = 3600;  

String sensorId = {"Unkown"};
int innerLoopDelayCount = 0;
int innerLoopDelayCountDefault = INNER_LOOP_DELAY_COUNT_DEFAULT;  // need to change this for production versipn
bool isZeroingInProgress(void);
void setZeroingInProgress(void);
virtual void measureLevel(void) = 0;

protected:
void publishLevel(int);

int sample;
bool zeroingInProgress;
String data = String(80);
bool firstTimeThrough = true;
int previousReading = 0;
boolean differential = false;  //Is this sensor to be read differentially one reading to the next.
uint64_t startOfMeasurement = 0;  // Start time of measurement in ms (we only want to delay 1s max per measurement)
    uint publishToSink = 0; //What stream to publish to - bit set: 1 = datatable, 2=iothub.


private:
void publish(uint);

};

#endif