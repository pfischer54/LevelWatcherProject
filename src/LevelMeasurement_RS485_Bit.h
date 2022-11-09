#ifndef LEVELMEASUREMENT_RS485_BIT_H
#define LEVELMEASUREMENT_RS485_BIT_H

#include "ModbusMaster.h"

extern ModbusMaster node;

class LevelMeasurement_RS485_Bit : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Bit(String sid, int slaveAddr, int sR, uint bit, boolean diff);
    void measureLevel();

private:
    int nodeAddr = {3}; // slave node address, defaults to 3.
    int startingRegister = {0};
    uint channelBit = 0; // active bit for this channel
};

#endif