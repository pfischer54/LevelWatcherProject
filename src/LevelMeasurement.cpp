#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
//#include <RunningAverage.h>
#include <CellularHelper.h>

LevelMeasurement::LevelMeasurement()
{
    zeroingInProgress = false;
    sampleReading = 0;
    data = "";
}
LevelMeasurement::LevelMeasurement(String sid) : LevelMeasurement()
{
    sensorId = sid;
}

LevelMeasurement::LevelMeasurement(String sid, boolean diff) :LevelMeasurement(sid)
{
    differential  = diff;
    
} 

bool LevelMeasurement::isZeroingInProgress(void)
{
    return zeroingInProgress;
}

void LevelMeasurement::setZeroingInProgress(void)
{
    zeroingInProgress = true;
    sample = 1; //Reset sample count for this sensor
}

void LevelMeasurement::publishLevel(int reading)
{

    Log.info("Sensor: " + sensorId  +  " Sample: " + String::format("%i", sample) + ", Reading: " + String::format("%u", reading));

    if (sample == LONG_SAMPLE_SIZE + 1)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {
            blinkLong(ZEROING_COMPLETED_BLINK_FREQUENCY); // Signal zeroing complete.
            Particle.publish(System.deviceID() + " zeroing completed for " + String("\"") + sensorId + String("\""), NULL, 600, PRIVATE);
            delay(1000);  //delay 1s so as to no overrun particle cloud
            zeroingInProgress = false;
        }
    }
// Check for differential reading: If differential is true, only send reading if it changed from last time.
        firstTimeThrough = false;
if (differential== true)
{
    if (reading == previousReading)
    {
        previousReading = reading;  //update reading;
    return;  //no need to publish, so exit.
    }
}
previousReading = reading;  //update reading;
//carry on

    // Trigger the integration

    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
    //TODO:
    //xxxString("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
    //xxx String("\"DT\":") + String("\"") + String::format("%u", System.millis()) + String("\",") +
    
    data = String("{") +
           String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", reading) + String("\",") +
           String("\"ZeroingInProgress\":") + String("\"") + String::format("%d", zeroingInProgress) +
           String("\"}");
    Particle.connect();                                 // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                        //Log.info(data);
                                                        //  Log.info(String::format("%f", waterLevelInMm));
                                                        //  Log.info(data);
    if (sample >= 0)
    {
        sample++;
    }
};