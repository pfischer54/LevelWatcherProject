#ifndef LEVELMEASUREMENT_RS485_ANALOGUE_H
#define LEVELMEASUREMENT_RS485_ANALOGUE_H

#include "ModbusMaster.h"

#define MAX_NO_OF_HOLDING_REGS 4 // max number with reading returned as 64bit int

const uint MODBUS_SLAVE_1 = 1;
const uint MODBUS_SLAVE_2 = 2;
const uint MODBUS_SLAVE_3 = 3;
const uint MODBUS_SLAVE_4 = 4;
const uint MODBUS_SLAVE_5 = 5;
const uint MODBUS_SLAVE_0 = 0;

const uint STARTING_REG_0 = 0;
const uint STARTING_REG_81H = 0x0081;
const uint STARTING_REG_200H = 0x0200;
const uint STARTING_REG_400H = 0x0400;
const uint STARTING_REG_403H = 0x0403;

const uint REGISTER_COUNT_1 = 1;
const uint REGISTER_COUNT_2 = 2;

const uint BIT_0 = 0b00000001; // Read bit 0
const uint BIT_1 = 0b00000010; // Read bit 1

// Gains and offsets used by Blynk

extern ModbusMaster node;

/// @brief Measurement class to measure analogue values.
/// @param sid  Sensor ID to identify the target sensor
/// @param bpid Blynk Virtual Pin ID
/// @param slaveAddr Modbus Slaved ID
/// @param sR Modbus Register
/// @param bit Bit to read 0-15
/// @param diff Publish every reading or only when reading changes from previous reading (plus regular heartbeats and readings to frame the transition)
/// @param sink Specify what endpoints to publish to: PUBLISH_2_BLYNK | PUBLISH_2_AZURE_TABLE | PUBLISH_2_AZURE_STREAM
/// @param o Sensor reading offset
/// @param g Sensor gain:  Published reading = (reading - o) * g
class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid, String bpid, int slaveAddr, int startingRegister, int numberOfRegistersToRead, boolean diff, uint sink, int o, float g);
    void measureReading();

private:
    int nodeAddr = {1}; // slave node address, defaults to 1.
    int startingRegister = {0};
    int numberOfRegistersToRead = {1};
};

#endif