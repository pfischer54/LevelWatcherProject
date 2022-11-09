#include "Particle.h"
#include <Adafruit_ADS1015.h>
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"

LevelMeasurement_4to20mA::LevelMeasurement_4to20mA(String sid, boolean diff) : LevelMeasurement(sid, diff)
{
    ads.setGain(GAIN_TWO); // GAIN_ONE for ...
    ads.begin();
}

void LevelMeasurement_4to20mA::measureLevel()
{
   int sampleReading = 0;
    startOfMeasurement = System.millis();       // mark  start time.
    sampleReading = ads.readADC_SingleEnded(0); // FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    publishLevel(sampleReading);
};