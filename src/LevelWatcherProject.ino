//Version info
//Includes sartup call to get and set zero offset in mm.
// This is done by publishing a startup event which triggers a function call to the device that includes
//the zero offest as a parameter in the function call.
//TODO how to make this device specific so that the function is only called on the device that is starting up?

// xxx 2
// This #include statement was automatically added by the Particle IDE.
#include <RunningAverage.h>
#include <CellularHelper.h>
#include <Adafruit_ADS1015.h>
#include "JsonParserGeneratorRK.h"

const unsigned long REBOOT_INTERVAL_IN_MS = 14*24*3600*1000;  // 14*24*3600*1000 Reboot every 14 days
const unsigned int DEFAULT_LOOP_DELAY_IN_MS = 60*60*1000; //60*60*1000; 1hour = (min = 60 = 1 hour)*(sec = 60 = 1 min)*(msec = 1000 = 1 sec)
const unsigned int REBOOT_DELAY_IN_MS = 15000;
const unsigned int ZEROING_LOOP_DELAY = 5000;  //Use shortish dealy while executing zeroing function
const unsigned int STARTUP_LOOP_DELAY = 10000;  //Use shortish dealy while waiting for startup handler to return and complete startup process
const int LONG_SAMPLE_SIZE = 24;  // number of measurements to average;
const int SHORT_SAMPLE_SIZE = 3;  // number of measurements to average;
const double FOUR_MA_OFFSET_IN_BITS = 6430; //3840.0;  //3840 for 120 Ohm, 6400 for 200 Ohm
const double MAX_16_BIT_ANALOGUE_BIT_VALUE = 32154; // 19200.0;  //19200 for 120 Ohm, 32000 for 200 Ohm-- see ndc datasheet on ADS1015
const double SENSOR_FULL_RANGE_IN_MM = 2000.0 ;

unsigned long rebootSync = 0;
bool resetFlag = false;
Adafruit_ADS1115 ads;
unsigned long loopDelay = DEFAULT_LOOP_DELAY_IN_MS;  //Loop delay default
int levelSensor = A0; //  Analogue input channel
int zeroVolt = A1;
int  zeroVoltSample = 0;
int  waterLevelSample = 0;
int sample = 1;
RunningAverage longAveragingArray(LONG_SAMPLE_SIZE);  //averaging bucket
RunningAverage shortAveragingArray(SHORT_SAMPLE_SIZE);  //averaging bucket
String data = String(80);
String zeroData = String(80);
String loopDelayData = String(80);
double waterLevelInMm;
double zeroOffsetInMm = 0.0;  //zeroing offset
int onboardLed = D7;  // Instead of writing D7 over and over again, we'll write led2
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.
bool zeroingInProgress = false;
bool startupCompleted = false;
JsonParserStatic<256, 20> parser;

//STARTUP(cellular_credentials_set("giffgaff.com", "giffgaff", "", NULL));
//STARTUP(cellular_credentials_set("3iot", "", "", NULL));  //globalM2M SIM starting 8953
//STARTUP(cellular_credentials_set("mokanix", "", "", NULL));
STARTUP(cellular_credentials_set("globaldata", "", "", NULL));  //globalM2M SIM starting 89234 or 89444


int setZero(String command)
{
    Serial.printlnf("Set Zero Function called from cloud");
    zeroOffsetInMm = 0.0;  //Reset zero offset to allow re-calculation
    longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
    zeroingInProgress = true;
    sample =1;
    return 0;
}

void startupHandler(const char *event, const char *data)
{
    // Handle the webhook response

    parser.clear();
    parser.addString(data);
    if (parser.parse()) {
        zeroOffsetInMm = parser.getReference().key("zeroOffsetInMm").valueFloat();
    }
    else {
        Serial.printlnf("error","could not parse json");
    }
    Serial.printlnf("zeroOffsetInMm (as stored on Azure): " + String::format("%4.1f", zeroOffsetInMm));
    zeroData = String("{") +
    String("\"ZeroOffsetInMm\":") +  String("\"") + String::format("%4.1f", zeroOffsetInMm)  +
    String("\"}");
    Particle.publish("Setting zeroOffsetInMm", zeroData, 600, PRIVATE);


    startupCompleted = true;  //We can now run loop
}

int setLoopDelay(String delay)
//Set loop delay in seconds
{
    loopDelay = atol(delay);
    Serial.printlnf("Loop Delay updated to: " + String::format("%u", loopDelay));
    loopDelayData = String("{") +
    String("\"LoopDelay\":") +  String("\"") + String::format("%u", loopDelay)  +
    String("\"}");
    Particle.publish("Loop Delay updated", loopDelayData, 600, PRIVATE);
    return 0;
}

int cloudResetFunction(String command) {
      Serial.printlnf("Restart triggered");
       resetFlag = true;
       rebootSync = millis();
       return 0;
}

void sos () {
  blinkShort(3);
  blinkLong(3);
  blinkShort(3);
}

void blink(unsigned long onTime)
{
    digitalWrite(onboardLed, HIGH);
    // We'll leave it on for 1 second...
    delay(onTime);
    // Then we'll turn it off...
    digitalWrite(onboardLed, LOW);
    delay(200);
}

void blinkLong(int times)
{
    for (int i = 0; i < times ; i++)
    {
        blink(600);
    }
}

void blinkShort(int times)
{
    for (int i = 0; i < times ; i++)
    {
        blink(200);
    }
}

void setup() {
    //
    Serial.printlnf("Startup: Running Setup");

    Particle.keepAlive(30);  //Needed for 3rd party SIMS

    //Register functions to control the electron
    Particle.function("CloudResetFunction", cloudResetFunction);
    Particle.function("SetLoopDelay", setLoopDelay);
    Particle.function("SetZero", setZero);

    // Subscribe to the webhook response event
    Particle.subscribe("hook-response/startup", startupHandler, MY_DEVICES);

    longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);  // Clear out averaging array
    shortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE);  // Clear out averaging array
    pinMode(onboardLed, OUTPUT);  //Setup activity led so we can blink it to show we're rolling...
    //   setADCSampleTime(ADC_SampleTime_3Cycles);
    //set ADC gain  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit=0.125mV
    //Setup ADC
    ads.setGain(GAIN_TWO);  //GAIN_ONE for ...
    ads.begin();
    Particle.publish("startup", NULL, 600, PRIVATE);  //TODO:  Specify and send sensor ID so as to retrieve correct offset.
}
//
// Main loop
//
void loop() {
  if ((millis() >=  REBOOT_INTERVAL_IN_MS)) {
    //Reboot regularly to freshen up
    // do things here  before reset and then push the button
    sos();
    Particle.publish("Debug","Reboot intiated", 300, PRIVATE);
    System.reset();
  }


    if ((resetFlag) && (millis() - rebootSync >=  REBOOT_DELAY_IN_MS)) {
      // do things here  before reset and then push the button
      sos();
      Particle.publish("Debug","Remote Reset Initiated", 300, PRIVATE);
      System.reset();
    }

    time_t time = Time.now();

    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();

    if (!startupCompleted)
    {
        blinkShort(4);  // Let know i'm waiting...
        delay(STARTUP_LOOP_DELAY);   //Wait a bit to  let syseem run ok
        return;
    }

    blinkShort (1);  //Signal normal running loop

//  System.sleep(10);
//  delay(8000);
    waterLevelSample = ads.readADC_SingleEnded(0); //FOR NDC setup -- ads.readADC_Differential_0_1() for ...;
    if (waterLevelSample > 1 and waterLevelSample <= MAX_16_BIT_ANALOGUE_BIT_VALUE) {
        //add sample if not an outlier
        //sometimes you get a duff reading, usually 0.  As we are 4-20mA must be greater than ...
        waterLevelInMm = (waterLevelSample - FOUR_MA_OFFSET_IN_BITS) * ( SENSOR_FULL_RANGE_IN_MM / (MAX_16_BIT_ANALOGUE_BIT_VALUE - FOUR_MA_OFFSET_IN_BITS)) - zeroOffsetInMm;
        longAveragingArray.addValue(waterLevelInMm);
        shortAveragingArray.addValue(waterLevelInMm);
    }
    Serial.printlnf(String::format("%i", sample)  + ", " + String::format("%u", waterLevelSample)  + ", " + String::format("%4.1f", waterLevelInMm) + ", " + String::format("%4.1f", longAveragingArray.getAverage()) + ", " + String::format("%4.1f", shortAveragingArray.getAverage()));

    if(sample == LONG_SAMPLE_SIZE)
    {
        sample = -1;  //  Hit the buffers no need to count anymore
        if(zeroingInProgress)
        {

            zeroOffsetInMm = longAveragingArray.getAverage();
            longAveragingArray.clear();
            longAveragingArray.fillValue(0.0, LONG_SAMPLE_SIZE);
            shortAveragingArray.clear();
            shortAveragingArray.fillValue(0.0, SHORT_SAMPLE_SIZE);
            zeroData = String("{") +
            String("\"ZeroOffsetInMm\":") +  String("\"") + String::format("%4.1f", zeroOffsetInMm)  +
            String("\"}");
            Particle.publish("saveZero", zeroData, 600, PRIVATE);
            Serial.printlnf("New zeroOffset (saved to cloud): " + zeroData);
            blinkLong(5);  // Signal zeroing complete.
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") +  String("\"") + String::format("%u", waterLevelSample)  + String("\",") +
           String("\"LsMm\":") +  String("\"") + String::format("%4.1f", waterLevelInMm) + String("\",") +
           String("\"LsAv\":") +  String("\"") + String::format("%4.1f", longAveragingArray.getAverage()) + String("\",") +
           String("\"LsShAv\":") +  String("\"") + String::format("%4.1f", shortAveragingArray.getAverage()) +
           String("\"}");
           Particle.connect();  // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("tickLevel", data, 600,  PRIVATE);  //TTL set to 3600s (may not yet be implemented)
//Serial.printlnf(data);
//  Serial.printlnf(String::format("%f", waterLevelInMm));
//  Serial.printlnf(data);

    if (sample > 0)
        ++sample;  //Increase sample count if on initial fill

    // Wait nn seconds
    if (zeroingInProgress)
        delay(ZEROING_LOOP_DELAY);  //Use shorter delay when averaging for zero...
    else
        delay(loopDelay);  //10 min: 600000, 1 min: 60000, 10 sec: 10000,
}
