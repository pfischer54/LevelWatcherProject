#include "Particle.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Bit.h"

LevelMeasurement_RS485_Bit::LevelMeasurement_RS485_Bit(String sid, String bpid, int slaveAddr, int sR, uint bit, boolean diff, uint sink) : LevelMeasurement(sid, bpid, diff, sink)
{
    nodeAddr = slaveAddr;
    startingRegister = sR;
    channelBit = bit;
    //AveragingArray(AVERAGING_SAMPLE_SIZE);  //averaging bucket
}

void LevelMeasurement_RS485_Bit::measureReading()
{
    int result;
    int sampleReading;

    sampleReading = 0;
    startOfMeasurement = System.millis(); // mark  start time.
    node.SetNodeAddr(nodeAddr);
    result = node.readHoldingRegisters(startingRegister, 1);

    // do something with data if read is successful
    if (result == node.ku8MBSuccess)
    {
        sampleReading = node.getResponseBuffer(0);
        Log.info("Sensor: " + sensorId + ": Success, Received data: " +  sampleReading);
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
