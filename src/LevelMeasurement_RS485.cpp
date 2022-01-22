#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485.h"
#include "LevelWatcher.h"
#include "UtilityFunctions.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include "ModbusMaster.h"

//#include <vector>
//std::vector<int> v{ 1, 2, 3 };  // v becomes {1, 2, 3}

LevelMeasurement_RS485::LevelMeasurement_RS485() : LevelMeasurement()
{
    // instantiate ModbusMaster object as slave ID 1
    ModbusMaster node(nodeAddr);
    // initialize Modbus communication baud rate
    node.begin(9600);     //pjf node.begin(57600);
    node.enableTXpin(D5); //D7 is the pin used to control the TX enable pin of RS485 driver
    //node.enableDebug();  //Print TX and RX frames out on Serial. Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.
    //while(!Serial.available()) Particle.process();
    Serial.println("Starting Modbus Transaction:");
};

LevelMeasurement_RS485::LevelMeasurement_RS485(String sid) : LevelMeasurement(sid)
{
    LevelMeasurement_RS485();
}

LevelMeasurement_RS485::LevelMeasurement_RS485(String sid, int slaveAddr) : LevelMeasurement(sid)
{
    nodeAddr = slaveAddr;
    LevelMeasurement_RS485();
}

void LevelMeasurement_RS485::measureLevel()
{
    int j, result;
    int rs485Data[10];

    waterLevelSampleReading = 0; 
    result = node.readHoldingRegisters(0x0, 1);

    Serial.println("");

    // do something with data if read is successful
    if (result == node.ku8MBSuccess)
    {
        Serial.print("Success, Received data: ");
        for (j = 0; j < 1; j++)
        {
            rs485Data[j] = node.getResponseBuffer(j);
            waterLevelSampleReading = rs485Data[j];
            Serial.print(waterLevelSampleReading);
            Serial.print(" ");
        }
        Serial.println("");
    }
    else
    {
        Serial.print("Failed, Response Code: ");
        Serial.print(result, HEX);
        Serial.println("");
        waterLevelSampleReading = 0;
    }

    publishLevel(waterLevelSampleReading);
};
