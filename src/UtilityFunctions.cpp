#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"

int onboardLed = D7;         // Instead of writing D7 over and over again, we'll write led2


void blink(unsigned long onTime)
{
    digitalWrite(onboardLed, HIGH);
    // We'll leave it on for 1 second...
    delay(onTime);
    // Then we'll turn it off...
    digitalWrite(onboardLed, LOW);
    delay(BLINK_OFF_DELAY_MS);
}

void blinkLong(int times)
{
    for (int i = 0; i < times; i++)
    {
        blink(LONG_BLINK_MS);
    }
}

void blinkShort(int times)
{
    for (int i = 0; i < times; i++)
    {
        blink(SHORT_BLINK_MS);
    }
}

void sos()
{
    blinkShort(3);
    blinkLong(3);
    blinkShort(3);
}

void initalizeAdc( Adafruit_ADS1115 ads)
{
      //   setADCSampleTime(ADC_SampleTime_3Cycles);
    //set ADC gain  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit=0.125mV
    //Setup ADC
  

    ads.setGain(GAIN_TWO); //GAIN_ONE for ...
    ads.begin();
 
}
uint8_t getSensorIndex(String sensorId)
{}

bool zeroingInProgress()
{
    return true;
}

int parseValue(char* data)
{
  parser.clear();
    parser.addString(data);
    if (parser.parse())
    
        return parser.getReference().key("zeroOffsetInMm").valueFloat();
    else
    return -1;
    }