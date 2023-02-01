#ifndef LEVELMEASUREMENT_4TO20MA_H
#define LEVELMEASUREMENT_4TO20MA_H

//#include "Particle.h"
//#include "LevelMeasurement.h"
//#include <RunningAverage.h>
//#include <CellularHelper.h>
//#include <Adafruit_ADS1015.h>
//#include "JsonParserGeneratorRK.h"
//#include <vector>

//https://store.ncd.io/product/2-channel-4-20-ma-current-loop-receiver-16-bit-ads1115-i2c-mini-module/
//This sets internal values for offsets used as described on above ncdio webpage.
//xxxconst double FOUR_MA_OFFSET_IN_BITS = 6430;         //3840.0;  //3840 for 120 Ohm, 6400 for 200 Ohm
const double MAX_16_BIT_ANALOGUE_BIT_VALUE = 32154; // 19200.0;  //19200 for 120 Ohm, 32000 for 200 Ohm-- see ndc datasheet on ADS1015 and 
//xxxconst double SENSOR_FULL_RANGE_IN_MM = 2000.0;



class LevelMeasurement_4to20mA: public LevelMeasurement
{

public:
LevelMeasurement_4to20mA();
LevelMeasurement_4to20mA(String sid);
LevelMeasurement_4to20mA(String sid, String bpid, boolean diff, uint sink, int o, float g);

void measureReading();

/* struct adcConfiguration
{
int channelNumber;  //adc channel number;
int adcAddress;  //addressof I2C device
int adcChannel;  //  adc channel number for this sensor
}; */


private:
Adafruit_ADS1115 ads;  // adc object
String data = String(80);

};

#endif