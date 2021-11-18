//Version info

//This is branch "master" ... and is WIP
//Includes startup call to get and set zero offset in mm.
// This is done by publishing a startup event which triggers a function call to the device that includes
//the zero offest as a parameter in the function call.
//TODO how to make this device specific so that the function is only called on the device that is starting up?

#include "Particle.h"
#include "LevelWatcher.h"
#include "LevelMeasurement.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include "JsonParserGeneratorRK.h"

int setAndSaveZero(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelay(String delay);
int cloudResetFunction(String command);
void sos();
void blink(unsigned long onTime);
void blinkLong(int times);
void blinkShort(int times);
void setup();
void loop();

unsigned long rebootSync = 0;
bool resetFlag = false;
Adafruit_ADS1115 ads;
unsigned long loopDelay = DEFAULT_LOOP_DELAY_IN_MS; //Loop delay default
int levelSensor = A0;                               //  Analogue input channel
int zeroVolt = A1;
int zeroVoltSample = 0;
int waterLevelSampleReading = 0;
int sample = 1;
// Two averaging buckets are provided, short and long averaging
RunningAverage longAveragingArray(LONG_SAMPLE_SIZE);   //averaging bucket
RunningAverage shortAveragingArray(SHORT_SAMPLE_SIZE); //averaging bucket
String data = String(80);
String zeroData = String(80);
String loopDelayData = String(80);
double waterLevelInMm;
double zeroOffsetInMm = 0.0; //zeroing offset
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
bool zeroingInProgress = false;
bool startupCompleted = false;
JsonParserStatic<256, 20> parser;

//Cellular constants
//String apn = "luner";
String apn = "3iot.com"; //globalM2M

//Objects etc
LevelMeasurement lm[1];

//DEBUG ON
// Use primary serial over USB interface for logging output
SerialLogHandler logHandler;
SYSTEM_THREAD(ENABLED);

STARTUP(cellular_credentials_set(apn, "", "", NULL));

int setAndSaveZero(String command)
{
    Log.info("Set Zero Function called from cloud");
    zeroOffsetInMm = 0.0; //Reset zero offset to allow re-calculation
    longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
    zeroingInProgress = true;
    sample = 1;
    return 0;
}

void startupHandler(const char *event, const char *data)
{
    // Handle the webhook response

    parser.clear();
    parser.addString(data);
    if (parser.parse())
    {
        zeroOffsetInMm = parser.getReference().key("zeroOffsetInMm").valueFloat();
    }
    else
    {
        Log.info("error: could not parse json");
    }
    Log.info("zeroOffsetInMm (as stored on Azure): " + String::format("%4.1f", zeroOffsetInMm));
    zeroData = String("{") +
               String("\"ZeroOffsetInMm\":") + String("\"") + String::format("%4.1f", zeroOffsetInMm) +
               String("\"}");
    Particle.publish("Setting zeroOffsetInMm", zeroData, 600, PRIVATE);

    startupCompleted = true; //We can now run loop
}

int setLoopDelay(String delay)
//Set loop delay in seconds
{
    loopDelay = atol(delay);
    Log.info("Loop Delay updated to: " + String::format("%u", loopDelay));
    loopDelayData = String("{") +
                    String("\"LoopDelay\":") + String("\"") + String::format("%u", loopDelay) +
                    String("\"}");
    Particle.publish("Loop Delay updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int cloudResetFunction(String command)
{
    Log.info("Restart triggered");
    resetFlag = true;
    rebootSync = millis();
    return 0;
}

void setup()
{
    //
    //DEBUG
    // Wait for a USB serial connection for up to 15 seconds
    waitFor(Serial.isConnected, 15000);
    delay(1000);
    Log.info("Startup: Running Setup");

    Particle.keepAlive(30); //Needed for 3rd party SIMS

    //Register functions to control the electron
    Particle.function("CloudResetFunction", cloudResetFunction);
    Particle.function("SetLoopDelay", setLoopDelay);
    Particle.function("SetAndSaveZero", setAndSaveZero);

    // Subscribe to the webhook response event
    Particle.subscribe(System.deviceID() + "/hook-response/Startup2/", startupHandler);

    longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);   // Clear out averaging array
    shortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE); // Clear out averaging array
    pinMode(ONBOARDLED, OUTPUT);                           //Setup activity led so we can blink it to show we're rolling...
    //   setADCSampleTime(ADC_SampleTime_3Cycles);
    //set ADC gain  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit=0.125mV
    //Setup ADC
    ads.setGain(GAIN_TWO); //GAIN_ONE for ...
    ads.begin();
    Particle.publish("Startup2", NULL, 600, PRIVATE); //TODO:  Specify and send sensor ID so as to retrieve correct offset.
}
//
// Main loop
//
void loop()
{
    if ((millis() >= REBOOT_INTERVAL_IN_MS))
    {
        //Reboot regularly to freshen up
        // do things here  before reset and then push the button
        sos();
        Particle.publish("Debug", "Reboot intiated", 300, PRIVATE);
        System.reset();
    }

    if ((resetFlag) && (millis() - rebootSync >= REBOOT_DELAY_IN_MS))
    {
        // do things here  before reset and then push the button
        sos();
        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        System.reset();
    }

    time_t time = Time.now();

    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();

    if (!startupCompleted)
    {
        // TODO:  add loop count and re-start if waiting too long as we missed the event.
        blinkShort(STARTUP_BLINK_FREQUENCY); // Let know i'm waiting...
        delay(STARTUP_LOOP_DELAY);           //Wait a bit to  let syseem run ok
        return;
    }

    blinkShort(NORMAL_LOOP_BLINK_FREQUENCY); //Signal normal running loop

    //  System.sleep(10);
    //  delay(8000);
    waterLevelSampleReading = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    if (waterLevelSampleReading > 1 and waterLevelSampleReading <= MAX_16_BIT_ANALOGUE_BIT_VALUE)
    {
        //add sample if not an outlier
        //sometimes you get a duff reading, usually 0.  As we are 4-20mA must be greater than ...
        waterLevelInMm = (waterLevelSampleReading - FOUR_MA_OFFSET_IN_BITS) * (SENSOR_FULL_RANGE_IN_MM / (MAX_16_BIT_ANALOGUE_BIT_VALUE - FOUR_MA_OFFSET_IN_BITS)) - zeroOffsetInMm;
        longAveragingArray.addValue(waterLevelInMm);
        shortAveragingArray.addValue(waterLevelInMm);
    }
    Log.info(String::format("%i", sample) + ", " + String::format("%u", waterLevelSampleReading) + ", " + String::format("%4.1f", waterLevelInMm) + ", " + String::format("%4.1f", longAveragingArray.getAverage()) + ", " + String::format("%4.1f", shortAveragingArray.getAverage()));

    if (sample == LONG_SAMPLE_SIZE)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {

            zeroOffsetInMm = longAveragingArray.getAverage();
            longAveragingArray.clear();
            longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
            shortAveragingArray.clear();
            shortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE);
            zeroData = String("{") +
                       String("\"ZeroOffsetInMm\":") + String("\"") + String::format("%4.1f", zeroOffsetInMm) +
                       String("\"}");
            Particle.publish("saveZero", zeroData, 600, PRIVATE);
            Log.info("New zeroOffset (saved to cloud): " + zeroData);
            blinkLong(5); // Signal zeroing complete.
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SensorId\":") + String("\"") + String("LS") + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", waterLevelSampleReading) + String("\",") +
           String("\"LsMm\":") + String("\"") + String::format("%4.1f", waterLevelInMm) + String("\",") +
           String("\"LsAv\":") + String("\"") + String::format("%4.1f", longAveragingArray.getAverage()) + String("\",") +
           String("\"LsShAv\":") + String("\"") + String::format("%4.1f", shortAveragingArray.getAverage()) +
           String("\"}");
    Particle.connect();                                 // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                        //Log.info(data);
                                                        //  Log.info(String::format("%f", waterLevelInMm));
                                                        //  Log.info(data);

    if (sample > 0)
        ++sample; //Increase sample count if on initial fill

    // Wait nn seconds
    if (zeroingInProgress)
        delay(ZEROING_LOOP_DELAY); //Use shorter delay when averaging for zero...
    else
        delay(loopDelay); //10 min: 600,000 1 min: 60,000 10 sec: 10,000
}
