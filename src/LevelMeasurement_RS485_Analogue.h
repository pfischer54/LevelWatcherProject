#ifndef LEVELMEASUREMENT_RS485_ANALOGUE_H
#define LEVELMEASUREMENT_RS485_ANALOGUE_H

#include "ModbusMaster.h"

extern ModbusMaster node;

class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid, int slaveAddr, boolean diff);
    void measureLevel();

private:
    int nodeAddr = {1}; //slave node address, defaults to 1.
};

#endif