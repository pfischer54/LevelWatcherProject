#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelMeasurement_RS485.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>



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
       sample = 1;  //Reset sample count for this sensor
}

void LevelMeasurement::publishLevel(int reading)
{
    
    Log.info(String::format("%i", sample) + ", " + String::format("%u", reading));

    if (sample == LONG_SAMPLE_SIZE + 1)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {
          blinkLong(ZEROING_COMPLETED_BLINK_FREQUENCY); // Signal zeroing complete.
          Particle.publish(System.deviceID() + " zeroing completed", NULL, 600, PRIVATE);
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
     time_t time = Time.now();
        CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
        //TODO:
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", reading) + String("\",") +
           String("\"ZeroingInProgress\":") + String("\"") + String::format("%d", zeroingInProgress) +
           String("\"}");
    Particle.connect();                                // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                       //Log.info(data);
                                                       //  Log.info(String::format("%f", waterLevelInMm));
                                                       //  Log.info(data);
 sample ++;
   
};