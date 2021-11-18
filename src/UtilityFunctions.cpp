#include "Particle.h"
#include "LevelWatcher.h"

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
