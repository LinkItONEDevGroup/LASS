/*
  Reference: 
  
    This is a demo to test MutichannelGasSensor library
    This code is running on Xadow-mainboard, and the I2C slave is Xadow-MutichannelGasSensor
    There is a ATmega168PA on Xadow-MutichannelGasSensor, it get sensors output and feed back to master.
    the data is raw ADC value, algorithm should be realized on master.
    
    please feel free to write email to me if there is any question 
    
    Jacky Zhang, Embedded Software Engineer
    qi.zhang@seeed.cc
    17,mar,2015
*/

//STATUS: basic work
//PIN setup: connect the sensor to grove I2C connector
#include <Wire.h>
#include "MutichannelGasSensor.h"

#define SENSOR_NH3 0
#define SENSOR_CO 1
#define SENSOR_NO2 2
#define SENSOR_C3H8 3
#define SENSOR_C4H10 4
#define SENSOR_CH4 5
#define SENSOR_H2 6
#define SENSOR_C2H5OH 7
float sensorValue[8];
float get_sensor_data_mgs(){
  
    

    sensorValue[SENSOR_NH3] = mutichannelGasSensor.measure_NH3();

    sensorValue[SENSOR_CO] = mutichannelGasSensor.measure_CO();

    sensorValue[SENSOR_NO2] = mutichannelGasSensor.measure_NO2();

    sensorValue[SENSOR_C3H8] = mutichannelGasSensor.measure_C3H8();

    sensorValue[SENSOR_C4H10] = mutichannelGasSensor.measure_C4H10();

    sensorValue[SENSOR_CH4] = mutichannelGasSensor.measure_CH4();

    sensorValue[SENSOR_H2] = mutichannelGasSensor.measure_H2();

    sensorValue[SENSOR_C2H5OH] = mutichannelGasSensor.measure_C2H5OH();


    if(sensorValue[SENSOR_NH3]>=0){
      Serial.print("The concentration of NH3 is ");
      Serial.print(sensorValue[SENSOR_NH3]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_CO]>=0){
      Serial.print("The concentration of CO is ");
      Serial.print(sensorValue[SENSOR_CO]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_NO2]>=0){
      Serial.print("The concentration of NO2 is ");
      Serial.print(sensorValue[SENSOR_NO2]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_C3H8]>=0){
      Serial.print("The concentration of C3H8 is ");
      Serial.print(sensorValue[SENSOR_C3H8]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_C4H10]>=0){
      Serial.print("The concentration of C4H10 is ");
      Serial.print(sensorValue[SENSOR_C4H10]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_CH4]>=0){
      Serial.print("The concentration of CH4 is ");
      Serial.print(sensorValue[SENSOR_CH4]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_H2]>=0){
      Serial.print("The concentration of H2 is ");
      Serial.print(sensorValue[SENSOR_H2]);
      Serial.println(" ppm");
    }

    if(sensorValue[SENSOR_C2H5OH]>=0){
      Serial.print("The concentration of C2H50H is ");
      Serial.print(sensorValue[SENSOR_C2H5OH]);
      Serial.println(" ppm");
    }
    Serial.print("CSV,");
    Serial.print(sensorValue[SENSOR_NH3]);    
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_CO]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_NO2]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_C3H8]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_C4H10]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_CH4]);
    Serial.print(",");
    Serial.print(sensorValue[SENSOR_H2]);
    Serial.print(",");
    Serial.println(sensorValue[SENSOR_C2H5OH]);    
}

void setup()
{
    Serial.begin(115200); // start serial for output
    Serial.println("power on!");

    
    mutichannelGasSensor.begin(0x04);//the default I2C address of the slave is 0x04
    //mutichannelGasSensor.changeI2cAddr(0x10);
    mutichannelGasSensor.doCalibrate();
    
    mutichannelGasSensor.powerOn();
    Serial.println("CSV,NH3,CO,NO2,C3H8,C4H10,CH4,H2,C2H5OH");
    
}

void loop()
{
  get_sensor_data_mgs();
  delay(1000);
  Serial.println("...........");
}
