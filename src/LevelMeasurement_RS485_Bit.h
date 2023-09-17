#ifndef LEVELMEASUREMENT_RS485_BIT_H
#define LEVELMEASUREMENT_RS485_BIT_H

#include "ModbusMaster.h"
// #include <RunningAverage.h>

extern ModbusMaster node1; 
extern ModbusMaster node5; 

/// @brief Measurement class to measure bit values.
/// @param sid  Sensor ID to identify the target sensor
/// @param bpid Blynk Virtual Pin ID
/// @param slaveAddr Modbus Slaved ID
/// @param sR Modbus Register
/// @param bit Bit to read 0-15
/// @param diff Publish every reading or only when reading changes from previous reading (plus regular heartbeats and readings to frame the transition)
/// @param sink Specify what endpoints to publish to: PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE | PUBLISH_2_AZURE_STREAM
/// @param msp  serial to use for master node
class LevelMeasurement_RS485_Bit : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Bit(String sid, String bpid, int slaveAddr, int sR, uint bit, boolean diff, uint sink, bool bm, String bmfs, uint msp);
    void measureReading();

private:
    int nodeAddr = {3}; // slave node address, defaults to 3.
    int startingRegister = {0};
    uint channelBit = 0; // active bit for this channel
    ModbusMaster *node; 
};
#endif