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

void LevelMeasurement_RS485::publishLevel(int reading)
{

    Log.info(String::format("%i", sample) + ", " + String::format("%u", reading));

    if (sample == LONG_SAMPLE_SIZE + 1)
    {
        sample = -1;           //  Hit the buffers no need to count anymore
        if (zeroingInProgress) //This is true if a cloud call has been made to set zero
        {
            blinkLong(ZEROING_COMPLETED_BLINK_FREQUENCY); // Signal zeroing complete.
            Particle.publish(System.deviceID() + " zeroing completed", NULL, 600, PRIVATE);
            zeroingInProgress = false;
        }
    }
    // Trigger the integration
    time_t time = Time.now();
    CellularHelperRSSIQualResponse rssiQual = CellularHelper.getRSSIQual();
    //TODO:
    data = String("{") +
           String("\"DT\":") + String("\"") + Time.format(time, TIME_FORMAT_ISO8601_FULL) + String("\",") +
           String("\"SensorId\":") + String("\"") + sensorId + String("\",") +
           String("\"SS\":") + String("\"") + String::format("rssi=%d, qual=%d", rssiQual.rssi, rssiQual.qual) + String("\",") +
           String("\"LsBits\":") + String("\"") + String::format("%u", reading) + String("\",") +
           String("\"ZeroingInProgress\":") + String("\"") + String::format("%d", zeroingInProgress) +
           String("\"}");
    Particle.connect();                                 // Not necessary but maybe this will help with poor connectivity issues as it will not return until device connected to cloud...
    Particle.publish("TickLevel2", data, 600, PRIVATE); //TTL set to 3600s (may not yet be implemented)
                                                        //Log.info(data);
                                                        //  Log.info(String::format("%f", waterLevelInMm));
                                                        //  Log.info(data);
    sample++;
};

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
