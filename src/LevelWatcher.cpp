//Version info

//This is branch "master" ... and is WIP
//Includes startup call to get and set zero offset in mm.
// This is done by publishing a startup event which triggers a function call to the device that includes
//the zero offest as a parameter in the function call.
//TODO how to make this device specific so that the function is only called on the device that is starting up?

#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement_4to20mA.h"
#include "LevelMeasurement_RS485.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>

// This turns off optimization for this file which makes it easier to debug.
// Otherwise you can't break on some lines, and some local variables won't
// be available.
#pragma GCC optimize ("O0")

//forward declarations
int measureZeroOffset(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelay(String delay);
int cloudResetFunction(String command);
int cloudResetFunction(String command);
void setup();
void loop();

String loopDelayData;
int measureZeroOffset(String command);
void startupHandler(const char *event, const char *data);
int setLoopDelay(String delay);
unsigned long rebootSync = 0;
bool resetFlag = false;
unsigned long loopDelay = DEFAULT_LOOP_DELAY_IN_MS; //Loop delay default
int startupCompleted = 0;

//Define sensor interfaces and objects and initialize sensor interfaces
LevelMeasurement_4to20mA lm0 = LevelMeasurement_4to20mA("LS");
LevelMeasurement_RS485 lm1 = LevelMeasurement_RS485("MS");
//xxxLevelMeasurement_RS485 lm2 = LevelMeasurement_RS485("TS");
LevelMeasurement *lm[2] = {&lm0, &lm1};  //xxx
//xxxLevelMeasurement *lm[1] = {&lm0};  //xxx

//Interaface objects
JsonParserStatic<256, 20> parser;

//Cellular constants
//String apn = "luner";
String apn = "3iot.com"; //globalM2M

//DEBUG ON
// Use primary serial over USB interface for logging output
SerialLogHandler logHandler;
//xxx SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC); //Used for debug? 

STARTUP(cellular_credentials_set(apn, "", "", NULL));

void setup()
{
    //
    //DEBUG
    // Wait for a USB serial connection for up to 15 seconds
    waitFor(Serial.isConnected, 15000);
    delay(1000);
    Log.info("Startup: Running Setup");
	Serial.begin(9600);

    Particle.keepAlive(30); //Needed for 3rd party SIMS

    //Register functions to control the electron
    Particle.function("CloudResetFunction", cloudResetFunction);
    Particle.function("SetLoopDelay", setLoopDelay);
    Particle.function("SetAndSaveZero", measureZeroOffset);

    // Subscribe to the webhook response event
    Particle.subscribe(System.deviceID() + "/hook-response/Initialize/", startupHandler);

    pinMode(STATUSLED, OUTPUT);                         //Setup activity led so we can blink it to show we're rolling...
    Particle.publish("Initialize", NULL, 600, PRIVATE); //TODO:  Specify and send sensor ID so as to retrieve correct offset.  Chaqnged name so as to not trigger LV3.
}
//
// Main loop
//
void loop()
{
    if ((millis() >= REBOOT_INTERVAL_IN_MS)  || (startupCompleted > STARTUP_LOOPS))
    {
        //Reboot regularly to freshen up
        // do things here  before reset and then push the button
        sos();
        Particle.publish("Debug", "Reboot intiated", 300, PRIVATE);
        System.reset();
    }

    if ((resetFlag) && (millis() - rebootSync >= REBOOT_DELAY_IN_MS))
    {
        // do things here  before reset and then push the button                                                                                                                                              on
        sos();
        Particle.publish("Debug", "Remote Reset Initiated", 300, PRIVATE);
        System.reset();
    }

    if (startupCompleted < STARTUP_LOOPS && startupCompleted > 0)
    //Keep waiting
    {
        blinkShort(STARTUP_BLINK_FREQUENCY); // Let know i'm waiting...
        delay(STARTUP_LOOP_DELAY);      
             //Wait a bit to  let syseem run ok
             startupCompleted++;
        return;
    }
 
     CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();

    lm[0]->measureLevel();
    lm[1]->measureLevel();
       //xxxlm[2]->measureLevel();

     // Wait nn seconds until all/any zeroing completed
    if (isAnyZeroingInProgress(lm))
    {
        blinkShort(ZEROING_IN_PROGRESS_LOOP_BLINK_FREQUENCY); //Signal zeroing running loop
        delay(ZEROING_LOOP_DELAY);                            //Use shorter delay when averaging for zero...
    }
    else
    {
        blinkShort(NORMAL_LOOP_BLINK_FREQUENCY); //Signal normal running loop
        delay(loopDelay);                        //10 min: 600,000 1 min: 60,000 10 sec: 10,000
    }
}

int measureZeroOffset(String command)
{
    int i;
    Log.info("ZeroingInProgress Function called from cloud");
    Particle.publish(System.deviceID() + " ZeroingInProgress for device " + command, NULL, 600, PRIVATE);
    i = atoi(command);
    if ((i > -1) && (i < NUMBER_OF_SENSORS))
    {
        lm[i]->setZeroingInProgress();
    }
    return 0;
}

void startupHandler(const char *event, const char *data)
{
    startupCompleted = -1 ; //We can now run loop
    Particle.publish(System.deviceID() + " initialized", NULL, 600, PRIVATE);
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