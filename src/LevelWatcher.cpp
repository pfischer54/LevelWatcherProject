// Version info
// This is branch "master" ... and is WIP
// synching
#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelMeasurement_RS485_Analogue.h"
#include "LevelMeasurement_RS485_Bit.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <CellularHelper.h>
#include "string.h"
#include "Serial5/Serial5.h"

// This turns off optimization for this file which makes it easier to debug.
// Otherwise you can't break on some lines, and some local variables won't
// be available.
#pragma GCC optimize("O0")

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); // Enable retained values feature

// DEBUG ON
//  Use primary serial over USB interface for logging output
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// forward declarations
int measureZeroOffset(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelaysFromCloud(const char *delays);
int setLoopDelaysWithTimeoutFromCloud(const char *delays);
int cloudResetFunction(String command);
int setBlynkBatchModeSize(const char *data);
int setBlynkPinToBatchMode(const char *data);
int setSensorDebugPublishState(const char *data);
int setAllSensorDebugPublishState(const char *data);

void setup();
void loop();
void setLoopDelays();

// globals

// Retained values
retained u_int LoopDelayDefaultCount[NUMBER_OF_MEASUREMENTS] = {50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000}; // Default cold start delay settings
retained u_int BlynkBatchModeSize = DEFAULT_BATCH_COUNT;                                                                        // Batch size if batching data for Blynk

//"normal"
unsigned long rebootSync = 0;
bool resetFlag = false;
bool startupCompleted = false;
unsigned long loopDelayTimeout = 0; // This will be the time for which the temporary loop delays run for before reverting
int startupLoopsCompleted = 0;
bool firstTimeThrough = true;

// RS485 setup
// Define the main node object. This controls the RS485 interface for all the slaves.
ModbusMaster node1 = ModbusMaster();
ModbusMaster node5 = ModbusMaster();

// Define sensor interfaces and objects and initialize sensor interfaces
// Tank levels

// From ncd.io: at 4mA the raw ADC value will be around 6430 - at 20mA the raw ADC value will be around 32154
LevelMeasurement_4to20mA lm0 = LevelMeasurement_4to20mA("LS", "V2", PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 2700, 0.0777484, false, "%.1f");
LevelMeasurement_RS485_Analogue lm1 = LevelMeasurement_RS485_Analogue("MS", "V3", MODBUS_SLAVE_1, STARTING_REG_0, REGISTER_COUNT_1, PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 0, 0.1, false, "%.1f", SERIAL_1);
LevelMeasurement_RS485_Analogue lm2 = LevelMeasurement_RS485_Analogue("TS", "V4", MODBUS_SLAVE_2, STARTING_REG_0, REGISTER_COUNT_1, PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 0, 0.1, false, "%.1f", SERIAL_1);
// Pressrising pump state
LevelMeasurement_RS485_Bit lm3 = LevelMeasurement_RS485_Bit("PP", "V1", MODBUS_SLAVE_3, STARTING_REG_081H, BIT_0, PUBLISH_DIFFERENTIAL_CHANGES, PUBLISH_2_BLYNK, true, "%.0g", SERIAL_1); // removed:  | PUBLISH_2_AZURE_STREAM  to save data :)
// FLow and volume
LevelMeasurement_RS485_Analogue lm4 = LevelMeasurement_RS485_Analogue("F1", "V5", MODBUS_SLAVE_4, STARTING_REG_400H, REGISTER_COUNT_2, PUBLISH_DIFFERENTIAL_CHANGES, PUBLISH_2_BLYNK, 0, 0.0167, true, "%.1f", SERIAL_5);
LevelMeasurement_RS485_Analogue lm5 = LevelMeasurement_RS485_Analogue("VM", "V0", MODBUS_SLAVE_4, STARTING_REG_200H, REGISTER_COUNT_2, PUBLISH_READINGS, PUBLISH_2_AZURE_TABLE | PUBLISH_2_BLYNK, 0, 1.0, false, "%.1f", SERIAL_5);
// Pressures
LevelMeasurement_RS485_Analogue lm6 = LevelMeasurement_RS485_Analogue("P1", "V6", MODBUS_SLAVE_6, STARTING_REG_004H, REGISTER_COUNT_1, PUBLISH_DIFFERENTIAL_CHANGES, PUBLISH_2_BLYNK, 0, 0.01, true, "%.2f", SERIAL_1);
LevelMeasurement_RS485_Analogue lm7 = LevelMeasurement_RS485_Analogue("P2", "V7", MODBUS_SLAVE_7, STARTING_REG_004H, REGISTER_COUNT_1, PUBLISH_DIFFERENTIAL_CHANGES, PUBLISH_2_BLYNK, 0, 0.01, true, "%.2f", SERIAL_1);
LevelMeasurement_RS485_Analogue lm8 = LevelMeasurement_RS485_Analogue("DP", "V8", MODBUS_SLAVE_8, STARTING_REG_004H, REGISTER_COUNT_1, PUBLISH_DIFFERENTIAL_CHANGES, PUBLISH_2_BLYNK, -1, 0.01, true, "%.2f", SERIAL_1);

LevelMeasurement *lm[NUMBER_OF_MEASUREMENTS] = {&lm0, &lm1, &lm2, &lm3, &lm4, &lm5, &lm6, &lm7, &lm8};

// Interface objects
JsonParserStatic<256, 20> parser;

// Cellular constants
String apn = "soracom.io";

// xxx SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC); // Used for debug?

STARTUP(cellular_credentials_set(apn, "", "", NULL));

void setup()
{

    // Specify logging level directly
    //
    // DEBUG
    // Wait for a USB serial connection for up to 15 seconds
    waitFor(Serial.isConnected, 15000);
    Serial.begin(9600);
    Log.info("Startup: Running Setup");

    Particle.keepAlive(30); // Needed for 3rd party SIMS

    // Register functions to control the electron
    Particle.function("CloudResetFunction", cloudResetFunction);
    Particle.function("SetLoopDelay", setLoopDelaysFromCloud);
    Particle.function("SetLoopDelayWithTimeout", setLoopDelaysWithTimeoutFromCloud);
    Particle.function("SetBlynkBatchModeSize", setBlynkBatchModeSize);
    Particle.function("SetBlynkPinToBatchMode", setBlynkPinToBatchMode);
    Particle.function("SetSensorDebugPublishState", setSensorDebugPublishState);
    Particle.function("SetAllSensorDebugPublishState", setAllSensorDebugPublishState);

    // Subscribe to the webhook startup2 response event
    // This handler is called by azure script response to Startup2 event published below.
    // Currently this is a placeholder so does nothing but there for future use.
    Particle.subscribe(System.deviceID() + "/hook-response/Startup2/", startupHandler);

    pinMode(STATUSLED, OUTPUT); // Setup activity led so we can blink it to show we're rolling...

    // RS485 start
    //  initialize Modbus communication baud rate and control pin
    //  Note: There is one node object that controls the RS485 interface for all the slaves.

    node1.setMasterSerialPort(1);
    node5.setMasterSerialPort(5);
    node1.begin(9600);
    node5.begin(9600);

    node1.enableTXpin(D5); // D5 is the pin used to control the TX enable pin of RS485 driver for Serial1
    node5.enableTXpin(D2); // D2 is the pin used to control the TX enable pin of RS485 driver for Serial5

    setLoopDelays(); // Set the delays from power on defaults or persisted values.
    Log.info("Setup Completed");
}
//
// Main loop
//
void loop()
{
    int sensorCount = 0;
    bool aSensorRead = false; //  will be true if at least one sensor read

    if (firstTimeThrough)
    {
        Particle.publish("Startup2", " V2023-10-28", 600, PRIVATE); // Device setup completed.  Publish/trigger this event as now ready to do any startup settings etc, currently NOOP.
        firstTimeThrough = false;
    }
    if ((System.millis() >= REBOOT_INTERVAL_IN_MS) || (startupLoopsCompleted > STARTUP_LOOPS))
    {
        // Reboot regularly to freshen up or if we missed startup acknowledgement from cloud
        //  do things here  before reset and then push the button
        sos();
        Particle.publish("Reboot Interval Reached Or Too Many Startup Loops", "Reboot intiated", 300, PRIVATE);
        startupCompleted = false;  // To be sure!
        startupLoopsCompleted = 0; // To be sure!
        System.reset();
    }

    if ((resetFlag) && (System.millis() - rebootSync >= REBOOT_DELAY_IN_MS))
    {
        // do things here  before reset and then push the button                                                                                                                                              on
        sos();

        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        startupCompleted = false;  // To be sure!
        startupLoopsCompleted = 0; // To be sure!
        resetFlag = false;         // To be sure!
        System.reset();
    }

    if (startupCompleted == false)
    // Keep waiting
    {

        Log.info("Startup Loop: Looping");
        blinkLong(STARTUP_BLINK_FREQUENCY); // Let know i'm waiting...
        delay(STARTUP_LOOP_DELAY);          // Wait a bit to  let system run
        startupLoopsCompleted++;
        return;
    }

    // Into main loop
    Log.info("Main Loop: Looping");
    // CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();

    for (sensorCount = 0; sensorCount < NUMBER_OF_MEASUREMENTS; sensorCount++)
    {
        aSensorRead = false;                                                                                     // reset
        if ((lm[sensorCount]->loopDelayCount >= lm[sensorCount]->loopDelay) && (lm[sensorCount]->loopDelay > 0)) // Set delay to -1 to disable measurement
        {
            lm[sensorCount]->measureReading();
            blinkVeryShort(OUTER_LOOP_BLINK_FREQUENCY);
            // delay(1s); // Delay a tiny bit so that we can see the outer look blink distincly
            lm[sensorCount]->loopDelayCount = 0; // reset loop count
            aSensorRead = true;                  // a sensor has been read
        }
        lm[sensorCount]->loopDelayCount++; // increment sensor publish delay count.
    }
    if (!aSensorRead)
        delay(1s); // make sure we have at least a 1s loop delay  TODO: allow this to be tuned

    blinkShort(INNER_LOOP_BLINK_FREQUENCY); // Signal normal running loop

    if (System.millis() > loopDelayTimeout)
        setLoopDelays(); // Time is up, reset loop delays to default value
}

void startupHandler(const char *event, const char *data)
{
    // NOOP at present.  Kept as placeholder for future use.
    startupCompleted = true; // We can now run main loop
    Particle.publish(System.deviceID() + " initialized", NULL, 600, PRIVATE);
}

//***************************************

// A function that takes a pointer to a string, a pointer to an end character
// It parses the string as a decimal number and returns it
// It also updates the end character
long parseDecimal(char **str)
{
    char *end;
    long result = strtol(*str, &end, 10); // Parse the string as a decimal number
    while (*end == ',')                   // Skip any commas
    {
        end++;
    }
    *str = end; // update to point to next token
    return result;
}

int setLoopDelaysFromCloud(const char *delays)
// Set loop delay count
{
    char tempchar[SIZE_OF_DELAY_ARRAY];
    strcpy(tempchar, delays); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx
    int index = 0;            // sensor delay index - initialize to -1 as call to parseDecimal increments index before first use

    String loopDelayData;
    String d = delays; // makes it easier to log and publish

    if (strlen(delays) == 0)
        return -1;

    while ((*buffptr) && (index < NUMBER_OF_MEASUREMENTS))
    {
        LoopDelayDefaultCount[index] = parseDecimal(&buffptr); // Set the default loop delays and update the variables;
        // Log.info("LoopDelayDefaultCount[%d]=%d\n", index, LoopDelayDefaultCount[index]);
        // Log.info("%s\n", buffptr);
        // Log.info("%u\n", LoopDelayDefaultCount[index] );
        index++;
    }

    setLoopDelays(); // Set the working loop delays

    Log.info("Loop Delay updated to: " + d);
    loopDelayData = String("{") +
                    String("\"LoopDelay\": ") + d +
                    String("\"}");
    Particle.publish("Loop Delay updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int setLoopDelaysWithTimeoutFromCloud(const char *params)
// Set loop delay count with a given time and then revert to previous
{
    String loopDelayData;
    char tempchar[SIZE_OF_DELAY_ARRAY];
    int index = 0;     // sensor delay index - initialize to -1 as call to parseDecimal increments index before first use
    String d = params; // makes it easier to log and publish

    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx

    if (strlen(params) == 0)
        return -1;

    while ((*buffptr) && (index < NUMBER_OF_MEASUREMENTS))
    {
        if (index == 0) // initial index
        {
            loopDelayTimeout = (parseDecimal(&buffptr) * 1000 * 60) + System.millis(); // Parse the first parameter and update the variables
            Log.info("LoopDelayTimeout: %lu\n", loopDelayTimeout);
        }
        else
        {
            lm[index - 1]->loopDelay = (int)parseDecimal(&buffptr); // Parse the other parameters and update the variables;
            Log.info("lm[%d]->loopDelay=%d\n", index - 1, lm[index - 1]->loopDelay);
        }
        index++;
    }

    Log.info("Loop Delays updated to: " + d);
    loopDelayData = String("{") +
                    String("\"LoopDelay\": ") + d +
                    String("\"}");
    Particle.publish("Loop Delay updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int cloudResetFunction(String command)
{
    Log.info("Restart triggered");
    resetFlag = true;
    rebootSync = System.millis();
    return 0;
}

void setLoopDelays()
{
    int i = 0;

    for (i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
        lm[i]->loopDelay = LoopDelayDefaultCount[i];
}

int setBlynkBatchModeSize(const char *data)
{
    if (strlen(data) == 0)
        return -1;

    BlynkBatchModeSize = atol(data);

    Serial.printlnf("BlynkBatchModeSize updated to: " + String::format("%u", BlynkBatchModeSize));
    String publishData = String("{") +
                         String("\"BlynkBatchModeSize\":") + String("\"") + String::format("%u", BlynkBatchModeSize) +
                         String("\"}");
    Particle.publish("BlynkBatchModeSize updated", publishData, 600, PRIVATE);
    return 0;
}

int setBlynkPinToBatchMode(const char *params)
{
    char tempchar[20];        // Plenty
    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx

    uint measurementIndex;
    bool OnOff;

    if (strlen(params) == 0)
        return -1;

    measurementIndex = parseDecimal(&buffptr);
    OnOff = (bool)parseDecimal(&buffptr);

    if (measurementIndex < NUMBER_OF_MEASUREMENTS)
        lm[measurementIndex]->blynkBatchMode = OnOff;

    Serial.printlnf("BlynkBatchMode for measurement %d is %d", measurementIndex, OnOff);
    String publishData = String("{") +
                         String("\"BlynkBatchMode for sensor ") + lm[measurementIndex]->sensorId + String(": ") + String::format("%d", OnOff) +
                         String("\"}");
    Particle.publish("BlynkBatchModeForPin updated", publishData, 600, PRIVATE);
    return 0;
}

int setSensorDebugPublishState(const char *params)
{
    char tempchar[20];        // Plenty
    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx

    uint measurementIndex;
    bool OnOff;

    if (strlen(params) == 0)
        return -1;

    measurementIndex = parseDecimal(&buffptr);
    OnOff = (bool)parseDecimal(&buffptr);

    if (measurementIndex < NUMBER_OF_MEASUREMENTS)
        lm[measurementIndex]->setDebug(OnOff);

    Serial.printlnf("Debug mode for sensor %s is %d", lm[measurementIndex]->sensorId.c_str(), OnOff);
    String publishData = String("{") +
                         String("\"Debug mode for sensor ") + lm[measurementIndex]->sensorId + String(": ") + String::format("%d", OnOff) +
                         String("\"}");
    Particle.publish("Debug mode for sensor updated", publishData, 600, PRIVATE);
    return 0;
}

int setAllSensorDebugPublishState(const char *params)
{
    char tempchar[20];        // Plenty
    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx

    bool OnOff;

    if (strlen(params) == 0)
        return -1;

    OnOff = (bool)parseDecimal(&buffptr);

    int i;
    for (i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
        lm[i]->setDebug(OnOff);

    Serial.printlnf("Debug mode for all sensors is %d", OnOff);
    String publishData = String("{") +
                         String("\"Debug mode is ") + String::format("%d", OnOff) +
                         String("\"}");
    Particle.publish("Debug mode for  all sensors updated", publishData, 600, PRIVATE);
    return 0;
}