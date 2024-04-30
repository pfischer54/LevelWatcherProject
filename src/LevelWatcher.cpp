// Version info
// This is branch "master" ... and is WIP // synching
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
int setSchedule(const char *delays);
int resetSchedulesToDefault(const char *params);
int setLoopDelaysWithTimeoutFromCloud(const char *delays);
int cloudResetFunction(String command);
int setBlynkBatchModeSize(const char *data);
int setBlynkPinToBatchMode(const char *data);
int setSensorDebugPublishState(const char *data);
int setAllSensorDebugPublishState(const char *data);
float timeNowAsDecimal();
void setup();
void loop();
void setDefaultLoopDelaysWorkingSet();
void setLoopDelaysWithRowFromSchedules(uint index);

// globals

// Retained values
retained uint loopDelayDefaultCount[NUMBER_OF_MEASUREMENTS] = {50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000}; // Default cold start delay settings
retained uint blynkBatchModeSize = DEFAULT_BATCH_COUNT;
retained float schedules[NUMBER_OF_SCHEDULES][NUMBER_OF_MEASUREMENTS + 2] = {{-1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, {-1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, {-1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

//"normal"
uint64_t uint64_t_max = std::numeric_limits<uint64_t>::max();
unsigned long rebootSync = 0;
bool resetFlag = false;
bool startupCompleted = false;
unsigned long loopDelayTimeout = uint64_t_max; // This will be the time for which the temporary loop delays run for before reverting
int startupLoopsCompleted = 0;
bool firstTimeThrough = true;
bool runningASchedule = false;
float tInt;

// RS485 setup
// Define the main node object. This controls the RS485 interface for all the slaves.
ModbusMaster node1 = ModbusMaster();
ModbusMaster node5 = ModbusMaster();

// Define sensor interfaces and objects and initialize sensor interfaces
// Tank levels

// From ncd.io: at 4mA the raw ADC value will be around 6430 - at 20mA the raw ADC value will be around 32154
LevelMeasurement_4to20mA lm0 = LevelMeasurement_4to20mA("LS", "V2", PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 2700, 0.0777484, false, "%.1f");
LevelMeasurement_RS485_Analogue lm1 = LevelMeasurement_RS485_Analogue("MS", "V3", MODBUS_SLAVE_1, STARTING_REG_0, REGISTER_COUNT_1, PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 0, 0.1, false, "%.1f", SERIAL_1);
LevelMeasurement_RS485_Analogue lm2 = LevelMeasurement_RS485_Analogue("TS", "V4", MODBUS_SLAVE_2, STARTING_REG_0, REGISTER_COUNT_1, PUBLISH_READINGS, PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE, 0, 0.1, false, "%.1f", SERIAL_1); // STARTING_REG_0 for running
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
    Particle.function("SetSchedule", setSchedule);
    Particle.function("ResetSchedulesToDefault", resetSchedulesToDefault);
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

    setDefaultLoopDelaysWorkingSet(); // Set the delays from power on defaults or persisted values.

    // set time zone to UTC  (=2 for Grece)
    Time.zone(0);

    Log.info("Setup Completed");
}
//
// Main loop
//
void loop()
{
    uint sensorCount = 0;
    uint scheduleNumber = 0;

    bool aSensorRead = false; //  will be true if at least one sensor read

    if (firstTimeThrough)
    {
        Particle.publish("Startup2", "V2024-04-30.1", 600, PRIVATE); // Device setup completed.  Publish/trigger this event as now ready to do any startup settings etc, currently NOOP.
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

        Log.info("Startup Loop: Looping at time %f", timeNowAsDecimal());
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
        aSensorRead = false;                                                                                                         // reset
        if ((lm[sensorCount]->loopDelayCount >= lm[sensorCount]->loopDelayWorkingSet) && (lm[sensorCount]->loopDelayWorkingSet > 0)) // Set delay to -1 to disable measurement
        {
            // xxx

            Log.info("Time sensor read - H: %d M: %d", Time.hour(), Time.minute());
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

    if (System.millis() > loopDelayTimeout && loopDelayTimeout != uint64_t_max) // TODO second condition redundant
    {
        setDefaultLoopDelaysWorkingSet(); // Time is up, reset loop delays to default values
        loopDelayTimeout = uint64_t_max;  // reset
        Log.info("Loop Delay Set back to working set");
        if (runningASchedule)
        {
            runningASchedule = false;
            Log.info("Schedule reset: %d", scheduleNumber);
        }
    }
    // Check for an active schedule...
    for (scheduleNumber = 0; scheduleNumber < NUMBER_OF_SCHEDULES; scheduleNumber++)
    {
        if (schedules[scheduleNumber][0] >= 0) // check schedules is active i.e. start time  (in decimal hours) >= 0
        {
            tInt = timeNowAsDecimal() - schedules[scheduleNumber][0]; // tInt is difference between time now in UTC and start time for this schedule in UTC in decimal hours.
            Log.info("tint: %f", tInt);
            if (tInt >= 0)
            {
                if (tInt < (schedules[scheduleNumber][1] / 60.0) && !runningASchedule)
                // timeout is in minutes (parameter 2)
                // Time now is > start of this schedule and time now < end of schedule and not running a schedule, then we are at a new schedule
                {
                    setLoopDelaysWithRowFromSchedules(scheduleNumber);
                    runningASchedule = true; // Set this schedule...
                    Log.info("Schedule set: %d", scheduleNumber);
                    break; // dont check any more schedules
                }
            }
        }
    }
}

void startupHandler(const char *event, const char *data)
{
    // NOOP at present.  Kept as placeholder for future use.
    startupCompleted = true; // We can now run main loop
    Particle.publish(System.deviceID() + " initialized", NULL, 600, PRIVATE);
}

//***************************************//

float timeNowAsDecimal()
{
    float h = Time.hour();
    float m = Time.minute();
    float s = Time.second();
    return (h + m / 60 + s / 3600); // Time in decimal hours
}

// This function takes a pointer to a string and parses the beginning of the string as a float.
// It then skips any commas or spaces following the number and updates the string pointer to point to the next character after the skipped characters.
// The parsed float is returned.

float parseFloat(char **str)
{
    char *end;
    double result = strtod(*str, &end); // Parse the string as a float number

    while (*end == ',' || *end == ' ') // Skip any commas or spaces
    {
        end++;
    }

    *str = end; // Update to point to next token
    return result;
}

// A function that takes a pointer to a string, a pointer to an end character
// It parses the string as a  long integer number and returns it
// It also updates the end character
long parseLongInt(char **str)
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

/**
 * @brief Set loop delay count from cloud
 *
 * This function takes a string of comma-separated delay values and sets the loop delay count accordingly.
 * The string is parsed as a series of long integers, each representing a delay count.
 *
 * @param delays A string of comma-separated delay values.
 * @return int Returns 0 if successful, -1 if the input string is empty.
 */
int setLoopDelaysFromCloud(const char *delays)
// Set loop delay count
{
    char tempchar[SIZE_OF_DELAY_ARRAY];
    strcpy(tempchar, delays); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx
    uint index = 0;           // sensor delay index - initialize to -1 as call to parseDecimal increments index before first use

    String loopDelayData;
    String d = delays; // makes it easier to log and publish

    if (strlen(delays) == 0)
        return -1;

    while ((*buffptr) && (index < NUMBER_OF_MEASUREMENTS))
    {
        loopDelayDefaultCount[index] = parseLongInt(&buffptr); // Set the default loop delays and update the variables;
        // Log.info("LoopDelayDefaultCount[%d]=%d\n", index, LoopDelayDefaultCount[index]);
        // Log.info("%s\n", buffptr);
        // Log.info("%u\n", LoopDelayDefaultCount[index] );
        index++;
    }

    setDefaultLoopDelaysWorkingSet(); // Set the working loop delays

    Log.info("Loop Delay updated to: " + d);
    loopDelayData = String("{") +
                    String("\"LoopDelay\": ") + d +
                    String("\"}");
    Particle.publish("Loop Delay updated", loopDelayData, 600, PRIVATE);
    return 0;
}

void setLoopDelaysWithRowFromSchedules(uint scheduleNumber)
{
    if (scheduleNumber < 0 || scheduleNumber >= NUMBER_OF_SCHEDULES)
    {
        Log.error("Invalid scheduleNumber");
        return;
    }

    for (uint i = 0; i < NUMBER_OF_MEASUREMENTS + 2; i++)
    {
        switch (i)
        {
        case 0:
            break;
        case 1:
        {
            loopDelayTimeout = schedules[scheduleNumber][1] * 1000 * 60 + System.millis(); // timeout given in minutes, convert to ms
            Log.info("loopDelayTimeout=%lu\n", loopDelayTimeout);
            break;
        }
        default:
        {
            lm[i - 2]->loopDelayWorkingSet = schedules[scheduleNumber][i]; // Set working set loop delay
            Log.info("lm[%d]->loopDelay=%d\n", i - 2, lm[i - 2]->loopDelayWorkingSet);
        }
        }
    }
}

int resetSchedulesToDefault(const char *params)
{
    for (uint i = 0; i < NUMBER_OF_SCHEDULES; i++)
    {
        for (uint j = 0; j < NUMBER_OF_MEASUREMENTS + 2; j++)
        {
            if (j == 1)
            {
                schedules[i][j] = 0;
            }
            else
            {
                schedules[i][j] = -1;
            }
        }
    }
     Particle.publish("Schedules reset", NULL, 600, PRIVATE);
    return 0;
}

int setSchedule(const char *params)
// Set loop delay count with a given time and then revert to previous
{
    String loopDelayData;
    char tempchar[SIZE_OF_DELAY_ARRAY];
    uint index = 0;    // sensor delay index - initialize to -1 as call to parseDecimal increments index before first use
    String d = params; // makes it easier to log and publish

    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx
    uint sn;                  // shcedule number

    if (strlen(params) == 0)
        return -1;

    while ((*buffptr) && (index < NUMBER_OF_MEASUREMENTS + 3))

    {
        if (index == 0) // initial index
        {
            // loopDelayTimeout is in ms, param is in minutes so convert to ms
            sn = parseLongInt(&buffptr); // Parse the first parameter and update the variables
            Log.info("Updating schedule=%d\n", sn);
        }
        else
        {
            schedules[sn][index - 1] = (float)parseFloat(&buffptr); // Parse the other parameters and update the variables;
            Log.info("schedules[%d][%d]=%f\n", sn, index - 1, schedules[sn][index - 1]);
        }
        index++;
    }

    Log.info("Schedule updated to: " + d);
    loopDelayData = String("{") +
                    String("\"Schedult\": ") + d +
                    String("\"}");
    Particle.publish("Schedule updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int setLoopDelaysWithTimeoutFromCloud(const char *params)
// Set loop delay count with a given time and then revert to previous
{
    String loopDelayData;
    char tempchar[SIZE_OF_DELAY_ARRAY];
    uint index = 0;    // sensor delay index - initialize to -1 as call to parseDecimal increments index before first use
    String d = params; // makes it easier to log and publish

    strcpy(tempchar, params); // need an mutable copy
    char *buffptr;            // probably redundant but just for now xx
    buffptr = tempchar;       // probably redundant but just for now xx

    if (strlen(params) == 0)
        return -1;

    while ((*buffptr) && (index < NUMBER_OF_MEASUREMENTS + 1))

    {
        if (index == 0) // initial index
        {
            // loopDelayTimeout is in ms, param is in minutes so convert to ms
            loopDelayTimeout = (parseLongInt(&buffptr) * 1000 * 60) + System.millis(); // Parse the first parameter and update the variables
            Log.info("LoopDelayTimeout=%lu\n", loopDelayTimeout);
        }
        else
        {
            lm[index - 1]->loopDelayWorkingSet = (int)parseLongInt(&buffptr); // Parse the other parameters and update the variables;
            Log.info("lm[%d]->loopDelay=%d\n", index - 1, lm[index - 1]->loopDelayWorkingSet);
        }
        index++;
    }

    Log.info("Loop Delays updated to: " + d);
    loopDelayData = String("{") +
                    String("\"LoopDelay\": ") + d +
                    String("\"}");
    Particle.publish("Loop Delays updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int cloudResetFunction(String command)
{
    Log.info("Restart triggered");
    resetFlag = true;
    rebootSync = System.millis();
    return 0;
}

void setDefaultLoopDelaysWorkingSet()
{
    uint i = 0;

    for (i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
        lm[i]->loopDelayWorkingSet = loopDelayDefaultCount[i];
}

int setBlynkBatchModeSize(const char *data)
{
    if (strlen(data) == 0)
        return -1;

    blynkBatchModeSize = atol(data);

    Serial.printlnf("BlynkBatchModeSize updated to: " + String::format("%u", blynkBatchModeSize));
    String publishData = String("{") +
                         String("\"BlynkBatchModeSize\":") + String("\"") + String::format("%u", blynkBatchModeSize) +
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

    measurementIndex = parseLongInt(&buffptr);
    OnOff = (bool)parseLongInt(&buffptr);

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

    measurementIndex = parseLongInt(&buffptr);
    OnOff = (bool)parseLongInt(&buffptr);

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

    OnOff = (bool)parseLongInt(&buffptr);

    uint i;
    for (i = 0; i < NUMBER_OF_MEASUREMENTS; i++)
        lm[i]->setDebug(OnOff);

    Serial.printlnf("Debug mode for all sensors is %d", OnOff);
    String publishData = String("{") +
                         String("\"Debug mode is ") + String::format("%d", OnOff) +
                         String("\"}");
    Particle.publish("Debug mode for  all sensors updated", publishData, 600, PRIVATE);
    return 0;
}