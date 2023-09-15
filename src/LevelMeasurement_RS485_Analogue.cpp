#include "Particle.h"
#include <Print64.h>
#include "LevelMeasurement.h"
#include "LevelMeasurement_RS485_Analogue.h"

LevelMeasurement_RS485_Analogue::LevelMeasurement_RS485_Analogue(String sid, String bpid, int slaveAddr, int sR, int nR, boolean diff, uint sink, int o, float g, bool bm, String bmfs, uint msp) : LevelMeasurement(sid, bpid, diff, sink, bm, bmfs)
{
    slaveNodeAddr = slaveAddr;
    startingRegister = sR;
    numberOfRegistersToRead = (nR < MAX_NO_OF_HOLDING_REGS) ? nR : MAX_NO_OF_HOLDING_REGS; // avoid fatality!
    offset = o;
    gain = g;
    if (msp == SERIAL_1)
        node = &node1; // yyy //temp
    else
        node = &node5; // yyy //temp
};

void LevelMeasurement_RS485_Analogue::measureReading()
{
    int j, result;
    int rs485Data[MAX_NO_OF_HOLDING_REGS];
    int64_t sampleReading = 0;

    startOfMeasurement = System.millis();                                             // mark  start time.
    (*node).setNodeAddr(slaveNodeAddr);                                               // yyy
    result = (*node).readHoldingRegisters(startingRegister, numberOfRegistersToRead); // yyy

    // do something with data if read is successful
    // TODO:  This must be buggy for 32bit and above :) ?
    if (result == (*node).ku8MBSuccess) // yyy
    {
        Log.info("Sensor: " + sensorId + ": Success, Received data: ");
        for (j = 0; j < numberOfRegistersToRead; j++) // This code only reads up to  4x 16bit register = 64 bit unsigned value
        {
            rs485Data[j] = (*node).getResponseBuffer(j); // yyy
            if (j == 0)
                sampleReading = rs485Data[j] <= 32767 ? rs485Data[j] : -(65536 - rs485Data[j]);
            else
                sampleReading = (sampleReading * 0x10000) + rs485Data[j]; // TODO 2s complement will fail with this method?
        }
        Log.info("Reading=%s", toString(sampleReading).c_str()); // Need special library function to handle llu (64bit) datatype
        publishLevel(sampleReading);
    }
    else
    {
        Log.info("Sensor: " + sensorId + ": Failed, Response Code: %x,", result);
        sampleReading = LLONG_MIN;
        //    if (result != (*node).ku8MBResponseTimedOut) // yyy
        //    {
        delay(1000ms);             // delay a bit to make sure sending sensor has sent all its stuff..
        (*node).flushReadBuffer(); // yyy
        (*node).reset();           // yyy
                                   //}
    }
}
