#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>


LevelMeasurement::LevelMeasurement()
{
      zeroingInProgress = false;
      waterLevelSampleReading = 0; 
      data = "";
}
LevelMeasurement::LevelMeasurement(String sid) : LevelMeasurement()
{
    sensorId = sid;
}

bool LevelMeasurement::isZeroingInProgress(void)
{
    return zeroingInProgress;
}

void LevelMeasurement::setZeroingInProgress(void)
{
 zeroingInProgress = true;
}