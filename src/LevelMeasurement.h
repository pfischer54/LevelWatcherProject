#ifndef LEVELMEASUREMENT_H
#define LEVELMEASUREMENT_H

// #include "Particle.h"
#include <RunningAverage.h>
// #include <CellularHelper.h>
#include "Adafruit_ADS1015.h"
// #include "JsonParserGeneratorRK.h"
// #include <vector>

// Two averaging buckets are provided, short and long averaging
const int LONG_SAMPLE_SIZE = 4;  // number of measurements to average for long term average;
const int SHORT_SAMPLE_SIZE = 3; // number of measurements to average for short term average;

const uint PUBLISH_2_AZURE_TABLE = 0x1;
const uint PUBLISH_2_AZURE_STREAM = 0x2;
const uint PUBLISH_2_THINGSPEAK = 0x4;
const uint PUBLISH_2_BLYNK = 0x8;
const uint CHECK_FOR_AVERAGE_USE = 0x10;

// Modbus
const uint SERIAL_1 = 1; // Use Serial1 Port for Modbus
const uint SERIAL_5 = 5; // Use Serial5 Port for Modbus

// Set channel type.
// Digital channels have an associated timer measuring ontime.

const uint ANALOGUE_CHANNEL = 0x0;
const uint DIGITAL_CHANNEL = 0x1;

const bool PUBLISH_READINGS = false;
const bool PUBLISH_DIFFERENTIAL_CHANGES = true;

const uint DIFFERENTIAL_READING_HEARTBEAT_COUNT = 100;

const int AVERAGING_SAMPLE_SIZE = 200; // number of measurements to average;

const int MAX_BLYNK_BATCH_MODE_BUFFER_SIZE = 864; // xxx864;   //max yte count for publish data https://docs.particle.io/reference/device-os/api/cloud-functions/overview-of-api-field-limits/

class LevelMeasurement
{
public:
    LevelMeasurement();
    LevelMeasurement(String sid);
    LevelMeasurement(String sid, String bid, boolean diff, uint sink, bool bm, String bmf);

    const int INNER_LOOP_DELAY_COUNT_DEFAULT = 3600;

    String sensorId = {"Unkown"};
    int loopDelayCount = 0;
    int loopDelayWorkingSet = INNER_LOOP_DELAY_COUNT_DEFAULT; // TODO: need to change this for production version and use retained variable
    bool blynkBatchMode = false;                    // Send this measurement to Blynk as a batch

    virtual void measureReading(void) = 0;
    void Add2BlynkBatchModeData(float);
    void setDebug(bool);

protected:
    void publishLevel(int);

    int sample;
    String data = String(80);
    bool firstTimeThrough = true;
    int previousReading = 0;
    boolean differential = false;    // Is this sensor to be read differentially one reading to the next.
    bool oneExtraSlice = false;      // send one more reading - relevant if change of state happening as sometimes readings get ouf of synch and this should ensure we get the right state transition (corrected if necessary)
    uint64_t startOfMeasurement = 0; // Start time of measurement in ms (we only want to delay 1s max per measurement)
    uint publishToSink = 0;          // What stream to publish to - bit set: 1 = datatable, 2=iothub.
    String blynkPinId;
    char blynkBatchModeData[MAX_BLYNK_BATCH_MODE_BUFFER_SIZE] = {0}; // Data string if batch mode used.
    uint blynkBatchModeCount = 0;                                    // Number of measurements so far
    String blynkBatchmodeReadingFormatString = "";                   // The string used to format this sensor measurement for transmission to Blunk
    float gain = 0.0;                                                // Gain  (Used only for Blynk data as this cannot be set by the app)
    int offset = 0;                                                  // Offset  (Used only for Blynk data as this cannot be set by the app)
    bool publishedAReading = false;
    uint diffHeartbeatReadingCount = 0;
    uint ChannelType = ANALOGUE_CHANNEL;
    ulong onTime;
    RunningAverage AveragingArray = RunningAverage(AVERAGING_SAMPLE_SIZE); // averaging bucket
    bool publishDebugData = false;

private:
    void publish(int);
};

#endif
