#ifndef LEVELMEASUREMENT_RS485_H
#define LEVELMEASUREMENT_RS485_H

#include "Particle.h"
#include "LevelMeasurement.h"
#include <RunningAverage.h>
#include <CellularHelper.h>
#include "JsonParserGeneratorRK.h"
#include <vector>



class LevelMeasurement_RS485: public LevelMeasurement
{

public:
LevelMeasurement_RS485();
LevelMeasurement_RS485(String sid);

void measureLevel(void);
void initializeInterfaceAndSensor(void);
void publishLevel(int reading);

struct rs485Configuration
{
int channelNumber;  //adc channel number;
int adcAddress;  //addressof I2C device
int adcChannel;  //  adc channel number for this sensor
};


private:

String data = String(80);

};

#endif