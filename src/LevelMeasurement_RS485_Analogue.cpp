#include "Particle.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Analogue.h"

LevelMeasurement_RS485_Analogue::LevelMeasurement_RS485_Analogue(String sid, String bpid, int slaveAddr, int sR, int nR, boolean diff, uint sink, int o, float g) : LevelMeasurement(sid, bpid, diff, sink)
{
    nodeAddr = slaveAddr;
    startingRegister = sR;
    numberOfRegistersToRead = (nR < MAX_NO_OF_HOLDING_REGS) ? nR: MAX_NO_OF_HOLDING_REGS;  //avoid fatality!
    offset = o;
    gain = g;
}

void LevelMeasurement_RS485_Analogue::measureReading()
{
    int j, result;
    int rs485Data[MAX_NO_OF_HOLDING_REGS];
    uint64_t sampleReading = 0;

    startOfMeasurement = System.millis(); // mark  start time.
    node.SetNodeAddr(nodeAddr);
    result = node.readHoldingRegisters(startingRegister, numberOfRegistersToRead);

    // do something with data if read is successful
    if (result == node.ku8MBSuccess)
    {
        Log.info("Sensor: " + sensorId + ": Success, Received data: ");
        for (j = 0; j < numberOfRegistersToRead; j++) // This code only reads 1x 16bit register
        {
            rs485Data[j] = node.getResponseBuffer(j);
            if (j == 0)
                sampleReading = rs485Data[j];
            else
                sampleReading = (sampleReading * 0x10000) + +rs485Data[j];
        }
        Log.info("Reading= %llu", sampleReading);
        publishLevel(sampleReading);
    }
    else
    {
        Log.info("Sensor: " + sensorId + ": Failed, Response Code: %x,", result);
        sampleReading = -1;
        if (result != node.ku8MBResponseTimedOut)
        {
            delay(1000ms); // delay a bit to make sure sending sensor has sent all its stuff..
            node.flushReadBuffer();
        }
    }
}
