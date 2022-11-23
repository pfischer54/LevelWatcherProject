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

extern ModbusMaster node;

class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid, int slaveAddr, int startingRegister, int numberOfRegistersToRead, boolean diff, uint sink);
    void measureReading();

private:
    int nodeAddr = {1}; // slave node address, defaults to 1.
    int startingRegister = {0};
    int numberOfRegistersToRead = {1};
};

#endif