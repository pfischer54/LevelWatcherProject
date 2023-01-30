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
const uint STARTING_REG_400H = 0x0400;
const uint STARTING_REG_403H = 0x0403;

const uint REGISTER_COUNT_1 = 1;
const uint REGISTER_COUNT_2 = 2;

const uint BIT_0 = 0b00000001;  // Read bit 0


extern ModbusMaster node;

class LevelMeasurement_RS485_Analogue : public LevelMeasurement
{

public:
    LevelMeasurement_RS485_Analogue(String sid, String bpid, int slaveAddr, int startingRegister, int numberOfRegistersToRead, boolean diff, uint sink);
    void measureReading();

private:
    int nodeAddr = {1}; // slave node address, defaults to 1.
    int startingRegister = {0};
    int numberOfRegistersToRead = {1};
};

#endif