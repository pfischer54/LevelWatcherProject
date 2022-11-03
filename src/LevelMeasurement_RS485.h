#ifndef LEVELMEASUREMENT_RS485_H
#define LEVELMEASUREMENT_RS485_H

//#include "Particle.h"
//#include "LevelMeasurement.h"
//#include <RunningAverage.h>
//#include <CellularHelper.h>
//#include "JsonParserGeneratorRK.h"
#include "ModbusMaster.h"
//#include <vector>

extern ModbusMaster node;

class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid);
    LevelMeasurement_RS485_Analogue(String sid, int slaveAddr);
    LevelMeasurement_RS485_Analogue(String sid, int slaveAddr, boolean diff);

    void measureLevel();

private:
    int nodeAddr = {1}; //slave node address, defaults to 1.
};

#endif