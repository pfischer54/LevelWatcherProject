#include "Particle.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>
#include "JsonParserGeneratorRK.h"
#include <vector>

// Two averaging buckets are provided, short and long averaging
const int LONG_SAMPLE_SIZE = 4;                    // number of measurements to average for long term average;
const int SHORT_SAMPLE_SIZE = 3;                    // number of measurements to average for short term average;
const double FOUR_MA_OFFSET_IN_BITS = 6430;         //3840.0;  //3840 for 120 Ohm, 6400 for 200 Ohm
const double MAX_16_BIT_ANALOGUE_BIT_VALUE = 32154; // 19200.0;  //19200 for 120 Ohm, 32000 for 200 Ohm-- see ndc datasheet on ADS1015
const double SENSOR_FULL_RANGE_IN_MM = 2000.0;


class LevelMeasurement
{

public:
bool zeroingInProgress;
void measureLevel(String sensorId);
void configureSensor(String sensorId, uint sensorType);
void updateAndSaveZero(String sensorId);
void readAndSetZeroAtStartup(String sensorId);

private:
Adafruit_ADS1115 ads;
int levelSensor = A0;                               //  Analogue input channel
int zeroVolt = A1;
int zeroVoltSample = 0;
int waterLevelSampleReading = 0;
int sample = 1;
// Two averaging buckets are provided, short and long averaging
RunningAverage longAveragingArray = RunningAverage(LONG_SAMPLE_SIZE);   //averaging bucket
RunningAverage shortAveragingArray = RunningAverage(SHORT_SAMPLE_SIZE); //averaging bucket
String data = String(80);
String zeroData = String(80);

double waterLevelInMm;
double zeroOffsetInMm = 0.0; //zeroing offset
int onboardLed = D7;         // Instead of writing D7 over and over again, we'll write led2
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.

JsonParserStatic<256, 20> parser;

};