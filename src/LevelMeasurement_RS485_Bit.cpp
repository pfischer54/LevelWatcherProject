#include "Particle.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Bit.h"


LevelMeasurement_RS485_Bit::LevelMeasurement_RS485_Bit(String sid, int slaveAddr, uint bit, boolean diff) : LevelMeasurement(sid, diff)
{
    nodeAddr = slaveAddr;
    channelBit = bit;

}

void LevelMeasurement_RS485_Bit::measureLevel()
{
    int j, result;
    int rs485Data[10];

    sampleReading = 0;
    startOfMeasurement = System.millis();  //mark  start time.
    node.SetNodeAddr(nodeAddr);
    result = node.readHoldingRegisters(0x0, 1);

    // do something with data if read is successful
    if (result == node.ku8MBSuccess)
    {
        Log.info("Sensor: " + sensorId + ": Success, Received data: ");
        for (j = 0; j < 1; j++)   //This code only reads 1x 16bit register
        {
            rs485Data[j] = node.getResponseBuffer(j);
            sampleReading = rs485Data[j];
            Log.info("Reading= %llu", sampleReading);
         }
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
