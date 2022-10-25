#include "Particle.h"
//#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485.h"
//#include "LevelWatcher.h"
//#include "UtilityFunctions.h"
//#include <RunningAverage.h>
//#include <CellularHelper.h>
//#include "ModbusMaster.h"

LevelMeasurement_RS485::LevelMeasurement_RS485(String sid) : LevelMeasurement(sid)
{
}

LevelMeasurement_RS485::LevelMeasurement_RS485(String sid, int slaveAddr) : LevelMeasurement(sid)
{
    nodeAddr = slaveAddr;
}

void LevelMeasurement_RS485::measureLevel()
{
    int j, result;
    int rs485Data[10];

    waterLevelSampleReading = 0;
    node.SetNodeAddr(nodeAddr);
    result = node.readHoldingRegisters(0x0, 1);

    // do something with data if read is successful
    if (result == node.ku8MBSuccess)
    {
        Log.info("Success, Received data: ");
        for (j = 0; j < 1; j++)
        {
            rs485Data[j] = node.getResponseBuffer(j);
            waterLevelSampleReading = rs485Data[j];
            Log.info("Reading= %d \r\n", waterLevelSampleReading);
         }
        Log.info("\r\n");
        publishLevel(waterLevelSampleReading);
    }
    else
    {
        Log.info("Failed, Response Code: %x, Sensor: " + sensorId + "\r\n", result);
        waterLevelSampleReading = -1;
        if (result != node.ku8MBResponseTimedOut)
        {
            delay(1000ms); // delay a bit to make sure sending sensor has sent all its stuff..
            node.flushReadBuffer();
        }
    }
}
