//Version info

//This is branch "master" ... and is WIP
//synching
#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelMeasurement_RS485.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <CellularHelper.h>


// This turns off optimization for this file which makes it easier to debug.
// Otherwise you can't break on some lines, and some local variables won't
// be available.
#pragma GCC optimize("O0")

//DEBUG ON
// Use primary serial over USB interface for logging output
SerialLogHandler logHandler;

//forward declarations
int measureZeroOffset(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelay(String delay);
int cloudResetFunction(String command);
int cloudResetFunction(String command);
void setup();
void loop();

//forward declarations
int measureZeroOffset(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelay(String delay);

//globals
String loopDelayData;
unsigned long rebootSync = 0;
bool resetFlag = false;
bool startupCompleted = false;
unsigned long loopDelay = DEFAULT_LOOP_DELAY_IN_MS; //Loop delay default
int startupLoopsCompleted = 0;
int innerLoopDelayCount = 0;
int innerLoopDelayCountDefault = INNER_LOOP_DELAY_COUNT;

// RS485 setup
//Define the main node object. This controls the RS485 interface for all the slaves.
ModbusMaster node = ModbusMaster();

//Define sensor interfaces and objects and initialize sensor interfaces
LevelMeasurement_4to20mA lm0 = LevelMeasurement_4to20mA("LS");
LevelMeasurement_RS485 lm1 = LevelMeasurement_RS485("MS", 1);
LevelMeasurement_RS485 lm2 = LevelMeasurement_RS485("TS", 2); //Set to slave addr 2.
LevelMeasurement *lm[3] = {&lm0, &lm1, &lm2};

//xxxLevelMeasurement *lm[2] = {&lm0, &lm1};

//Interaface objects
JsonParserStatic<256, 20> parser;

//Cellular constants

String apn = "luner";
//String apn = "3iot.com"; //globalM2M

//xxx SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC); //Used for debug?

STARTUP(cellular_credentials_set(apn, "", "", NULL));

void setup()
{
    //
    //DEBUG
    // Wait for a USB serial connection for up to 15 seconds
    waitFor(Serial.isConnected, 15000);
    Log.info("Startup: Running Setup");
    Serial.begin(9600);
    Particle.keepAlive(30); //Needed for 3rd party SIMS

    //Register functions to control the electron
    Particle.function("CloudResetFunction", cloudResetFunction);
    Particle.function("SetLoopDelay", setLoopDelay);
    Particle.function("SetAndSaveZero", measureZeroOffset);

    // Subscribe to the webhook startup2 response event
    //This handler is called by azure script response to Startup2 event published below.
    //Currently this is a placeholder so does nothing but there for future use.
    Particle.subscribe(System.deviceID() + "/hook-response/Startup2/", startupHandler);

    pinMode(STATUSLED, OUTPUT);                       //Setup activity led so we can blink it to show we're rolling...
    Particle.publish("Startup2", NULL, 600, PRIVATE); //Device setup completed.  Publish/trigger this event as now ready to do any startup settings etc, currently NOOP.

    //RS485 start
    // initialize Modbus communication baud rate and control pin
    // Note: There is one node object that controls the RS485 interface for all the slaves.

    node.begin(9600);     //pjf node.begin(57600);
    node.enableTXpin(D5); //D7 is the pin used to control the TX enable pin of RS485 driver
    //node.enableDebug();  //Print TX and RX frames out on Serial. Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.
}
//
// Main loop
//
void loop()
{
    if ((millis() >= REBOOT_INTERVAL_IN_MS) || (startupLoopsCompleted > STARTUP_LOOPS))
    {
        //Reboot regularly to freshen up or if we missed startup acknowledgement from cloud
        // do things here  before reset and then push the button
        sos();
        Particle.publish("Debug", "Reboot intiated", 300, PRIVATE);
        startupCompleted = false;  // To be sure!
        startupLoopsCompleted = 0; //To be sure!
        System.reset();
    }

    if ((resetFlag) && (millis() - rebootSync >= REBOOT_DELAY_IN_MS))
    {
        // do things here  before reset and then push the button                                                                                                                                              on
        sos();

        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        startupCompleted = false;  // To be sure!
        startupLoopsCompleted = 0; //To be sure!
        resetFlag = false;         //To be sure!
        System.reset();
    }

    if (startupCompleted == false)
    //Keep waiting
    {
        blinkLong(STARTUP_BLINK_FREQUENCY); // Let know i'm waiting...
        delay(STARTUP_LOOP_DELAY);          //Wait a bit to  let system run
        startupLoopsCompleted++;
        return;
    }

    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();

    if ((innerLoopDelayCount >= innerLoopDelayCountDefault) || isAnyZeroingInProgress(lm))
    {
        lm[0]->measureLevel();
        lm[1]->measureLevel();
        lm[2]->measureLevel();
         blinkShort(OUTER_LOOP_BLINK_FREQUENCY);
        delay(1s); // Delay a tiny bit so that we can see the outer look blink distincly
       innerLoopDelayCount = 0; //reset loop count
    }

    // Wait nn seconds until all/any zeroing completed
    if (isAnyZeroingInProgress(lm))
    {
        blinkLong(ZEROING_IN_PROGRESS_LOOP_BLINK_FREQUENCY); //Signal zeroing running loop
        delay(ZEROING_LOOP_DELAY);                           //Use shorter delay when averaging for zero...
    }
    else
    {
        blinkVeryShort(INNER_LOOP_BLINK_FREQUENCY); //Signal normal running loop
        delay(loopDelay);                           //10s by default
        innerLoopDelayCount++;                      //inc inner loop count
    }
}

int measureZeroOffset(String command)
{
    int i;
    Log.info("ZeroingInProgress Function called from cloud");
    i = atoi(command);
    if ((i > -1) && (i < NUMBER_OF_SENSORS))
    {
        Particle.publish(System.deviceID() + " ZeroingInProgress for sensor " + lm[i]->sensorId, NULL, 600, PRIVATE);
        lm[i]->setZeroingInProgress();
    }
    return 0;
}

void startupHandler(const char *event, const char *data)
{
    //NOOP at present.  Kept as placeholder for future use.
    startupCompleted = true; //We can now run main loop
    Particle.publish(System.deviceID() + " initialized", NULL, 600, PRIVATE);
}

int setLoopDelay(String delay)
//Set loop delay count (default is n * 10s inner loop)
{
    innerLoopDelayCountDefault = atol(delay);
    Log.info("Loop Delay updated to: " + String::format("%u", loopDelay * innerLoopDelayCountDefault));
    loopDelayData = String("{") +
                    String("\"LoopDelay\":") + String("\"") + String::format("%u", loopDelay * innerLoopDelayCountDefault) +
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
