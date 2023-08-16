#include "Particle.h"

#include "Print64.h"

SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler;

void setup() {

}

void loop() {
    Log.info("millis=%s", toString(System.millis()).c_str());
    delay(1000);
}
