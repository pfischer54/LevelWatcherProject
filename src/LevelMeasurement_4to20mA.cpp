#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>

//#include <vector>
//std::vector<int> v{ 1, 2, 3 };  // v becomes {1, 2, 3}

LevelMeasurement_4to20mA::LevelMeasurement_4to20mA() : LevelMeasurement(){};

LevelMeasurement_4to20mA::LevelMeasurement_4to20mA(String sid) : LevelMeasurement(sid)
{
    //setADCSampleTime(ADC_SampleTime_3Cycles);
    //set ADC gain  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit=0.125mV
    //Setup ADC
    ads.setGain(GAIN_TWO); //GAIN_ONE for ...
    ads.begin();
}

void LevelMeasurement_4to20mA::measureLevel()
{
    waterLevelSampleReading = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    publishLevel(waterLevelSampleReading);
};