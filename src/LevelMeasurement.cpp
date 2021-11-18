#include "Particle.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include "LevelMeasurement.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include "JsonParserGeneratorRK.h"
#include <vector>
std::vector<int> v{ 1, 2, 3 };  // v becomes {1, 2, 3}

void LevelMeasurement::measureLevel(String sensorId)
{
      waterLevelSampleReading = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    if (waterLevelSampleReading > 1 and waterLevelSampleReading <= MAX_16_BIT_ANALOGUE_BIT_VALUE)
    {
        //add sample if not an outlier
        //sometimes you get a duff reading, usually 0.  As we are 4-20mA must be greater than ...
        waterLevelInMm = (waterLevelSampleReading - FOUR_MA_OFFSET_IN_BITS) * (SENSOR_FULL_RANGE_IN_MM / (MAX_16_BIT_ANALOGUE_BIT_VALUE - FOUR_MA_OFFSET_IN_BITS)) - zeroOffsetInMm;
        longAveragingArray.addValue(waterLevelInMm);
        shortAveragingArray.addValue(waterLevelInMm);
    }
    Log.info(String::format("%i", sample) + ", " + String::format("%u", waterLevelSampleReading) + ", " + String::format("%4.1f", waterLevelInMm) + ", " + String::format("%4.1f", longAveragingArray.getAverage()) + ", " + String::format("%4.1f", shortAveragingArray.getAverage()));

    if (sample == LONG_SAMPLE_SIZE)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {

            zeroOffsetInMm = longAveragingArray.getAverage();
            longAveragingArray.clear();
            longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
            shortAveragingArray.clear();
            shortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE);
            zeroData = String("{") +
                       String("\"ZeroOffsetInMm\":") + String("\"") + String::format("%4.1f", zeroOffsetInMm) +
                       String("\"}");
            Particle.publish("saveZero", zeroData, 600, PRIVATE);
            Log.info("New zeroOffset (saved to cloud): " + zeroData);
            blinkLong(5); // Signal zeroing complete.
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
     time_t time = Time.now();
        CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SensorId\":") + String("\"") + String("LS") + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", waterLevelSampleReading) + String("\",") +
           String("\"LsMm\":") + String("\"") + String::format("%4.1f", waterLevelInMm) + String("\",") +
           String("\"LsAv\":") + String("\"") + String::format("%4.1f", longAveragingArray.getAverage()) + String("\",") +
           String("\"LsShAv\":") + String("\"") + String::format("%4.1f", shortAveragingArray.getAverage()) +
           String("\"}");
    Particle.connect();                                // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                       //Log.info(data);
                                                       //  Log.info(String::format("%f", waterLevelInMm));
                                                       //  Log.info(data);

   
};