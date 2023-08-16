#include "Particle.h"
#include <Print64.h>
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Analogue.h"

LevelMeasurement_RS485_Analogue::LevelMeasurement_RS485_Analogue(String sid, String bpid, int slaveAddr, int sR, int nR, boolean diff, uint sink, int o, float g) : LevelMeasurement(sid, bpid, diff, sink)
{
    nodeAddr = slaveAddr;
    startingRegister = sR;
    numberOfRegistersToRead = (nR < MAX_NO_OF_HOLDING_REGS) ? nR : MAX_NO_OF_HOLDING_REGS; // avoid fatality!
    offset = o;
    gain = g;
}

void LevelMeasurement_RS485_Analogue::measureReading()
{
    int j, result;
    int rs485Data[MAX_NO_OF_HOLDING_REGS];
    int64_t sampleReading = 0;

    startOfMeasurement = System.millis(); // mark  start time.
    node.SetNodeAddr(nodeAddr);
    result = node.readHoldingRegisters(startingRegister, numberOfRegistersToRead);

    // do something with data if read is successful
    //TODO:  This must be buggy for 32bit and above :) ?
    if (result == node.ku8MBSuccess)
    {
        Log.info("Sensor: " + sensorId + ": Success, Received data: ");
        for (j = 0; j < numberOfRegistersToRead; j++) // This code only reads up to  4x 16bit register = 64 bit unsigned value
        {
            rs485Data[j] = node.getResponseBuffer(j);
            if (j == 0)
                sampleReading = rs485Data[j] <= 32767 ? rs485Data[j] : -(65536 - rs485Data[j]);
            else
                sampleReading = (sampleReading * 0x10000) + rs485Data[j]; // TODO 2s complement will fail with this method?
        }
        Log.info("Reading=%s", toString(sampleReading).c_str());  //Need special library function to handle llu (64bit) datatype
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
            node.Reset();
        }
    }
}
