#ifndef LEVELWATCHER_H
#define LEVELWATCHER_H 
#define STATUSLED D7 //D0, D1 used fro I2C,  D6 and D7 used for debugger

const int NUMBER_OF_SENSORS = 2;  //number of sensors to scan

//Sensor types enum

#define SENSOR_0 "LS";
#define SENSOR_1 "MS";
#define SENSOR_2 "TS";

const unsigned long REBOOT_INTERVAL_IN_MS = 14 * 24 * 3600 * 1000; // 14*24*3600*1000 Reboot every 14 days
const unsigned int DEFAULT_LOOP_DELAY_IN_MS = 10 * 1 * 1000;      //60*60*1000; 1hour = (min = 60 = 1 hour)*(sec = 60 = 1 min)*(msec = 1000 = 1 sec)
const unsigned int REBOOT_DELAY_IN_MS = 15000;
const unsigned int ZEROING_LOOP_DELAY = 5000;  //Use shortish dealy while executing zeroing function
const unsigned int STARTUP_LOOP_DELAY = 10000; //Use shortish dealy while waiting for startup handler to return and complete startup process
const  int INNER_LOOP_DELAY_COUNT = 360;  // Default loop count - each inner loop takes 10s:  60 == 10min, 360 == 1h
const int STARTUP_LOOPS = 20;  //Number of loops allowed for startup processing

//LEDs timings etc
const int LONG_BLINK_MS = 600;
const int SHORT_BLINK_MS = 200;
const int VERY_SHORT_BLINK_MS = 20;
const int BLINK_OFF_DELAY_MS = 200;
const int STARTUP_BLINK_FREQUENCY = 6;
const int INNER_LOOP_BLINK_FREQUENCY = 1;
const int OUTER_LOOP_BLINK_FREQUENCY = 3;
const int ZEROING_IN_PROGRESS_LOOP_BLINK_FREQUENCY = 2;
const int ZEROING_COMPLETED_BLINK_FREQUENCY = 5;

extern JsonParserStatic<256, 20> parser;


#endif


