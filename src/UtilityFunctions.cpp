#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"

void blink(unsigned long onTime)
{
    digitalWrite(STATUSLED, HIGH);
    // We'll leave it on for 1 second...
    delay(onTime);
    // Then we'll turn it off...
    digitalWrite(STATUSLED, LOW);
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

uint8_t getSensorIndex(String sensorId)
{
    return 0; //xxx
}

bool isAnyZeroingInProgress(LevelMeasurement * lm[])
{
    int i;
    
    //Checks if zeroing is going on  for any device.

    for (i = 0; i < NUMBER_OF_SENSORS; i++)
    {
        if (lm[i]->isZeroingInProgress() == true)
            return true;
      }
            return false;
}

    int parseValue(char *data)
    {
        parser.clear();
        parser.addString(data);
        if (parser.parse())

            return parser.getReference().key("zeroOffset").valueFloat();
        else
            return -1;
    }