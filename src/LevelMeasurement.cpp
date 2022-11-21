#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <CellularHelper.h>

LevelMeasurement::LevelMeasurement()
{
    zeroingInProgress = false;
    data = "";
}
LevelMeasurement::LevelMeasurement(String sid) : LevelMeasurement()
{
    sensorId = sid;
}

LevelMeasurement::LevelMeasurement(String sid, boolean diff, uint sink) : LevelMeasurement(sid)
{
    differential = diff;
    publishToSink = sink;
}

bool LevelMeasurement::isZeroingInProgress(void)
{
    return zeroingInProgress;
}

void LevelMeasurement::setZeroingInProgress(void)
{
    zeroingInProgress = true;
    sample = 1; // Reset sample count for this sensor
}

void LevelMeasurement::publish(uint reading)
{
    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
    // TODO:
    // xxxString("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
    // xxx String("\"DT\":") + String("\"") + String::format("%u", System.millis()) + String("\",") +

    Particle.connect(); // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...

    if (publishToSink & PUBLISH_2_AZURE_TABLE)
    {
        data = String("{") +
               String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
               String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
               String("\"LsBits\":") + String("\"") + String::format("%u", reading) + String("\",") +
               String("\"ZeroingInProgress\":") + String("\"") + String::format("%d", zeroingInProgress) +
               String("\"}");
        Particle.publish("TickLevel2", data, 600, PRIVATE); // TTL set to 3600s (may not yet be implemented)
                                                            // Log.info(data);
                                                            //   Log.info(String::format("%f", waterLevelInMm));
                                                            //   Log.info(data);
    }


    if (publishToSink & PUBLISH_2_AZURE_STREAM)
    {
        data = String("{") +
               String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
               String("\"State\":") + String("\"") + String::format("%u", reading) +
               String("\"}");
        Particle.publish("PressuringPumpStatus", data, PRIVATE);
    }


    if (publishToSink & PUBLISH_2_THINGSPEAK)
    {
        data = String("{") +
               String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
               String("\"State\":") + String("\"") + String::format("%u", reading) +
               String("\"}");
        Particle.publish("thingSpeakWrite_A0", "{ \"1\": \"" + String::format("%u", reading) + "\", \"k\": \"JNCR3IEAN13USRSZ\" }", 60, PRIVATE);
    } 

}

void LevelMeasurement::publishLevel(int reading)
{
    uint64_t timeTakenForMeasurement;

    Log.info("Sensor: " + sensorId + " Sample: " + String::format("%i", sample) + ", Reading: " + String::format("%u", reading));

    if (sample == LONG_SAMPLE_SIZE + 1)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) // This is true if a cloud call has been made to set zero
        {
            blinkLong(ZEROING_COMPLETED_BLINK_FREQUENCY); // Signal zeroing complete.
            Particle.publish(System.deviceID() + " zeroing completed for " + String("\"") + sensorId + String("\""), NULL, 600, PRIVATE);
            zeroingInProgress = false;
        }
    }
    // Check for differential reading: If differential is true, only send reading if it changed from last time.

    if (firstTimeThrough | !differential)
    {
        publish(reading);
        firstTimeThrough = false;
    }
    else
    {
        if (reading == previousReading)
        {
            if (oneExtraSlice) // send last reading one more time in case of timing skew, lost readings, etc.
            {
                publish(reading); // publish one more reading
                oneExtraSlice = false;
            }
            return; // now return.
        }
        else
        {
            publish(previousReading);        // First publish previous reading, i.e. reading valid up until this point in time
            delay(DIFFERENTIAL_DELAY_IN_MS); // wait 1s so as not to overload particle cloud
            publish(reading);                // Now publish reading follwing transition to new reading.
            oneExtraSlice = true;            // we will send out one more reading to deal with time skews and missed packets/
        }
    }

    previousReading = reading; // update reading;

    if (sample >= 0)
    {
        sample++;
    }

    timeTakenForMeasurement = System.millis() - startOfMeasurement; // how long did this take?
    if (timeTakenForMeasurement < 1000)
        delay(1000 - timeTakenForMeasurement); // make sure we are not publishing events at a rate > 1/sec
};