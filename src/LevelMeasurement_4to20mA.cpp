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


LevelMeasurement_4to20mA::LevelMeasurement_4to20mA() {};

LevelMeasurement_4to20mA::LevelMeasurement_4to20mA(String sid): LevelMeasurement(sid)
{
     //setADCSampleTime(ADC_SampleTime_3Cycles);
    //set ADC gain  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit=0.125mV
    //Setup ADC
    ads.setGain(GAIN_TWO); //GAIN_ONE for ...
    ads.begin();
}

void LevelMeasurement_4to20mA::publishLevel(int reading)
{
      reading = 0; //ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    
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
 
   
};


void LevelMeasurement_4to20mA::measureLevel()
{
      waterLevelSampleReading = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
   
    publishLevel(waterLevelSampleReading);
   
};