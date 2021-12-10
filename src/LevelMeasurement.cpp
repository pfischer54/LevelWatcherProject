#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>

//#include <vector>
//std::vector<int> v{ 1, 2, 3 };  // v becomes {1, 2, 3}

LevelMeasurement::LevelMeasurement(String sid)
{
    sensorId = sid;
    //xxxlongAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);   // Clear out averaging array
    //xxxshortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE); // Clear out averaging array
}

void LevelMeasurement::measureLevel( Adafruit_ADS1115 ads, const char* sensorId)
{
      waterLevelSampleReading = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    if (waterLevelSampleReading > 1 and waterLevelSampleReading <= MAX_16_BIT_ANALOGUE_BIT_VALUE)
    {
        //add sample if not an outlier
        //sometimes you get a duff reading, usually 0.  As we are 4-20mA must be greater than ...
        //The following now calculated clientside:
        //waterLevel = (waterLevelSampleReading - FOUR_MA_OFFSET_IN_BITS) * (SENSOR_FULL_RANGE_IN_MM / (MAX_16_BIT_ANALOGUE_BIT_VALUE - FOUR_MA_OFFSET_IN_BITS)) - zeroOffsetInMm;
        //xxxlongAveragingArray.addValue(waterLevelSampleReading);
        //xxxshortAveragingArray.addValue(waterLevelSampleReading);
    }
    Log.info(String::format("%i", sample) + ", " + String::format("%u", waterLevelSampleReading));

    if (sample == LONG_SAMPLE_SIZE + 1)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {
          blinkLong(ZEROING_COMPLETED_BLINK_FREQUENCY); // Signal zeroing complete.
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
     time_t time = Time.now();
        CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
        //TODO:
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SensorId\":") + String("\"") + String("LS") + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", waterLevelSampleReading) + String("\",") +
           String("\"ZeroingInProgress\":") + String("\"") + String::format("%d", lm[0].zeroingInProgress) +
           String("\"}");
    Particle.connect();                                // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                       //Log.info(data);
                                                       //  Log.info(String::format("%f", waterLevelInMm));
                                                       //  Log.info(data);
 
 
   
};