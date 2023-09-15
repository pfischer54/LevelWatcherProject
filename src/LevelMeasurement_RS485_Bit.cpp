#include "Particle.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Bit.h"

LevelMeasurement_RS485_Bit::LevelMeasurement_RS485_Bit(String sid, String bpid, int slaveAddr, int sR, uint bit, boolean diff, uint sink, bool bm, String bmf, uint msp) : LevelMeasurement(sid, bpid, diff, sink, bm, bmf)
{
    nodeAddr = slaveAddr;
    startingRegister = sR;
    channelBit = bit;
    if (msp == SERIAL_1)
        node = &node1; // yyy //temp
    else
        node = &node5; // yyy //temp
    ;
}

void LevelMeasurement_RS485_Bit::measureReading()
{
    int result;
    int sampleReading;

    sampleReading = 0;
    startOfMeasurement = System.millis();                       // mark  start time.
    (*node).setNodeAddr(nodeAddr);                              // yyy
    result = (*node).readHoldingRegisters(startingRegister, 1); // yyy

    // do something with data if read is successful
    if (result == (*node).ku8MBSuccess) // yyy
    {
        sampleReading = (*node).getResponseBuffer(0); // yyy
        Log.info("Sensor: " + sensorId + ": Success, Received data: %d" + sampleReading);
        publishLevel(sampleReading);
    }
    else
    {
        Log.info("Sensor: " + sensorId + ": Failed, Response Code: %x,", result);
        sampleReading = -1;
        //    if (result != (*node).ku8MBResponseTimedOut) // Not sure what this was doing here 2023-09-15
        //   {
        delay(1000ms);             // delay a bit to make sure sending sensor has sent all its stuff..
        (*node).flushReadBuffer(); // yyy
        (*node).reset();           // added reset.  2023-09-15
        //   }
    }
}
