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

LevelMeasurement::LevelMeasurement(String sid, String bpid, boolean diff, uint sink) : LevelMeasurement(sid)
{
    differential = diff;                                  // Publish every measurement or only changes
    publishToSink = sink;                                 // Who to publish to
    blynkPinId = bpid;                                    // Blynk needs it's own virtual pin designation for each signal
    AveragingArray.clear();
    AveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
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

void LevelMeasurement::publish(int reading)
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
               String("\"LsBits\":") + String("\"") + String::format("%d", reading) + String("\",") +
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
               String("\"State\":") + String("\"") + String::format("%d", reading) +
               String("\"}");
        Particle.publish("PressuringPumpStatus", data, PRIVATE);
    }

    if (publishToSink & PUBLISH_2_THINGSPEAK)
    {
        data = String("{") +
               String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
               String("\"State\":") + String("\"") + String::format("%d", reading) +
               String("\"}");
        Particle.publish("thingSpeakWrite", "{ \"1\": \"" + String::format("%d", reading) + "\", \"k\": \"JNCR3IEAN13USRSZ\" }", 60, PRIVATE);
    }

    if (publishToSink & PUBLISH_2_BLYNK)
    {
        float scaledReading = 0.0;
        if (gain == 0) // This is a bit or integer stream
        {
            data = String("{") +
                   String("\"") + blynkPinId + String("\":\"") + +String::format("%d", reading) +
                   String("\"}");
            Particle.publish("BlynkWrite" + blynkPinId, data, PRIVATE);
        }
        else // This stream needs sclaling and offsetting... publish as a float
        {
            scaledReading = (reading - offset) * gain;
            data = String("{") +
                   String("\"") + blynkPinId + String("\":\"") + +String::format("%f", scaledReading) +
                   String("\"}");
            Particle.publish("BlynkWrite" + blynkPinId, data, PRIVATE);
        }
    }
    if (publishToSink & CHECK_FOR_AVERAGE_USE)
    {
        AveragingArray.addValue(reading);
    }

}

void LevelMeasurement::publishLevel(int reading)
{
    uint64_t timeTakenForMeasurement;
    publishedAReading = false; // start by assuming we are not publishing

    Log.info("Sensor: " + sensorId + " Sample: " + String::format("%i", sample) + ", Reading: " + String::format("%d", reading));

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

    if (firstTimeThrough || !differential)
    {
        publish(reading);
        firstTimeThrough = false;
        publishedAReading = true; // we published
    }
    else // This is not first time through and this is a differential reading
    {
        if (reading == previousReading)
        {
            if (oneExtraSlice) // send last reading one more time in case of timing skew, lost readings, etc.
            {
                publish(reading); // publish one more reading
                oneExtraSlice = false;
                diffHeartbeatReadingCount = 0; // we published so reset count
            }
        }
        else
        {
            publish(previousReading);        // First publish previous reading, i.e. reading valid up until this point in time
            delay(DIFFERENTIAL_DELAY_IN_MS); // wait 1s so as not to overload particle cloud
            publish(reading);                // Now publish reading following transition to new reading.
            oneExtraSlice = true;            // we will send out one more reading to deal with time skews and missed packets/
            publishedAReading = true;        // we published
        }

        if (!publishedAReading)
        {
            if (diffHeartbeatReadingCount >= DIFFERENTIAL_READING_HEARTBEAT_COUNT)
            {
                publish(reading);              // publish a heartbeat reading
                diffHeartbeatReadingCount = 0; // reset count
            }
            else
                diffHeartbeatReadingCount++; //  we did not publish a reading so increment count
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