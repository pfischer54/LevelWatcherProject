#ifndef LEVELMEASUREMENT_RS485_ANALOGUE_H
#define LEVELMEASUREMENT_RS485_ANALOGUE_H

#include "ModbusMaster.h"

#define MAX_NO_OF_HOLDING_REGS 4 // max number with reading returned as 64bit int

extern ModbusMaster node;

class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid, int slaveAddr, int startingRegister, int numberOfRegistersToRead, boolean diff, uint sink);
    void measureLevel();

private:
    int nodeAddr = {1}; // slave node address, defaults to 1.
    int startingRegister = {0};
    int numberOfRegistersToRead = {1};
};

#endif