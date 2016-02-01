# arduino-sht
Unofficial repository for Sensirion Humidity and temperature sensor support on Arduino

## Supported sensors:
- SHTC1
- SHT3x-DIS (I2C)
- SHT3x-ARP (ratiometric analog voltage output)

## Installation

Download arduino-sht either via git or from the releases page and place it in the Arduino/libraries directory. After restarting the Arduino IDE, you will get menu items under libraries and examples.

## Integrating it into your sketch (Example for SHTC1)

Assuming you installed the library as described above, the following steps are necessary:

1. Import the Wire library like this: From the menu bar, select Sketch > Import Library > Wire
1. Import the arduino-sht library like this: From the menu bar, select Sketch > Import Library > arduino-sht
1. Create an instance of the `SHTC1` class
2. In `setup()`, make sure to init the Wire library with `Wire.init()`
3. If you want to use the serial console, remember to initialize the Serial library with `Serial.begin(9600)`
1. Call `shtc1.readSample()` in the `loop()` function, which reads a temperature & humidity sample from the sensor
2. Use `shtc1.getHumidity()` and `shtc1.getTemperature()` to get the values form the last sample

*Important:* `getHumidity()` and `getTemperature()` do *not* read a new sample from the sensor, but return the values read last. To read a new sample, make sure to call `readSample()`

### Sample code
```c++
#include <Wire.h>

#include <shtc1.h>

SHTC1 shtc1;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  shtc1.readSample();
  Serial.print("SHTC1:\n");
  Serial.print("  RH: ");
  Serial.print(shtc1.getHumidity(), 2);
  Serial.print("\n");
  Serial.print("  T:  ");
  Serial.print(shtc1.getTemperature(), 2);
  Serial.print("\n");

  delay(1000);
}
```
