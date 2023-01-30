#ifndef LEVELMEASUREMENT_RS485_BIT_H
#define LEVELMEASUREMENT_RS485_BIT_H

#include "ModbusMaster.h"
#include <RunningAverage.h>

const int AVERAGING_SAMPLE_SIZE = 200; // number of measurements to average;

extern ModbusMaster node;

/// @brief Measurement class to measure bit values.
/// @param sid  Sensor ID to identify the target sensor
/// @param bpid Blynk Virtual Pin ID
/// @param slaveAddr Modbus Slaved ID
/// @param sR Modbus Register 
/// @param bit Bit to read 0-15
/// @param diff Publish every reading or only when reading changes from previous reading (plus regular heartbeats and readings to frame the transition)
/// @param sink Specify what endpoints to publish to: PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE | PUBLISH_2_AZURE_STREAM
class LevelMeasurement_RS485_Bit : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Bit(String sid, String bpid, int slaveAddr, int sR, uint bit, boolean diff, uint sink);
    void measureReading();

private:
    int nodeAddr = {3}; // slave node address, defaults to 3.
    int startingRegister = {0};
    uint channelBit = 0;                                                   // active bit for this channel
    RunningAverage AveragingArray = RunningAverage(AVERAGING_SAMPLE_SIZE); // averaging bucket
};
#endif