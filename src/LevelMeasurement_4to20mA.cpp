#include "Particle.h"
#include <Adafruit_ADS1015.h>
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"

LevelMeasurement_4to20mA::LevelMeasurement_4to20mA(String sid, String bpid, boolean diff, uint sink, int o, float g, bool bm, String bmfs) : LevelMeasurement(sid, bpid, diff, sink, bm, bmfs)
{
    ads.setGain(GAIN_TWO); // GAIN_ONE for ...
    ads.begin();
    offset = o;
    gain = g;
}

void LevelMeasurement_4to20mA::measureReading()
{
    int sampleReading = 0;
    startOfMeasurement = System.millis();       // mark  start time.
    sampleReading = ads.readADC_SingleEnded(0); // FOR NDC setup -- ads.readADC_Differential_0_1() for ...
    // From ncd.io: at 4mA the raw ADC value will be around 6430 - at 20mA the raw ADC value will be around 32154
    if ((sampleReading != 65535)) // xxx  && (sampleReading > 6400)) // 65535 means no interface present or 24V failure and value must be > 4mA Offset i.e. about 6400
        publishLevel(sampleReading);
    else
      if (publishDebugData)
        {
            char status[] = "{\"debug\":\"Sensor: %s failed, Sample Reading: %d\"}";
            char errorMsg[sizeof(status) + 16];
            snprintf(errorMsg, sizeof(errorMsg), status, sensorId.c_str(), sampleReading);
            Particle.publish("DEBUG", errorMsg, 60, PRIVATE);
        }
          // Log.info("4to20ma Reading Failed For Sensor %s\n", sensorId.c_str());
        // Log.info("4to20ma Reading Failed For Sensor " + sensorId);
        Log.info("\nSensor: " + sensorId + ": Failed.  4to20mA Sample Reading: %d", sampleReading);
        
};