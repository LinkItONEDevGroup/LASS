#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

#define DELAY 60000

#define WATER_WIRE_BUS 2
#define SOIL_WIRE_BUS 3

#define ETAPE_PIN A0
#define ETAPE_NUMSAMPLES 15
#define ETAPE_SERIESRESISTOR 560
#define ETAPE_FUDGE 0.3
int samples[ETAPE_NUMSAMPLES];

OneWire waterWire(WATER_WIRE_BUS);
OneWire soilWire(SOIL_WIRE_BUS);

DallasTemperature water_temp(&waterWire);
DallasTemperature soil_temp(&soilWire);

float etape_liquid_level = 0;

float water_temperature = 0;

float soil_temperature = 0;

float waterlevel;

LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial mySerial(4, 5); // UNO (RX, TX)

void read_water_temp() {
    water_temp.requestTemperatures();
    water_temperature = water_temp.getTempCByIndex(0);

    if(water_temperature == -127) {
      water_temperature = 0;
    }

    Serial.print("Water Temperature: ");
    Serial.print(water_temperature);
    Serial.println(" *C ");
}

void read_soil_temp() {
    soil_temp.requestTemperatures();
    soil_temperature = soil_temp.getTempCByIndex(0);
    
    if(soil_temperature == -127) {
      soil_temperature = 0;
    }
    
    Serial.print("Soil Temperature: ");
    Serial.print(soil_temperature);
    Serial.println(" *C ");
}

void read_etape() {
     uint8_t i;
     float average;
     float lastwaterlevel=0;
    // take N samples in a row, with a slight delay
    for (i=0; i< ETAPE_NUMSAMPLES; i++) {
    samples[i] = analogRead(ETAPE_PIN);
    delay(10);
    }
    // average all the samples out
    average = 0;
    for (i=0; i< ETAPE_NUMSAMPLES; i++) {
    average += samples[i];
    }
    average /= ETAPE_NUMSAMPLES;
    //Serial.print("Average analog reading ");
    //Serial.println(average);
    // convert the value to resistance
    average = 1023 / average - 1;
    average = ETAPE_SERIESRESISTOR / average;
    //Serial.print("Sensor resistance ");
    //Serial.println(average);
    waterlevel = 0;
    waterlevel= -1 * 0.006958 * average + 11.506958 + ETAPE_FUDGE;
    etape_liquid_level = waterlevel;

    if(waterlevel < 0) {
      waterlevel = 0;
    }
    
    Serial.print("Water level (inches) ");
    Serial.print(waterlevel);
    Serial.print(", (cm) ");
    Serial.println(waterlevel * 2.54);
}

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600);
    water_temp.begin();
    soil_temp.begin();
    
    lcd.begin();
    lcd.backlight();
}

void loop() {
  read_etape();
  read_water_temp();
  read_soil_temp();

  mySerial.print(water_temperature);
  mySerial.print(",");
  mySerial.print(soil_temperature);
  mySerial.print(",");
  mySerial.println(waterlevel * 2.54);
  
  lcd.clear();
  lcd.print(water_temperature);
  lcd.print(" C ,");
  lcd.print(soil_temperature);
  lcd.print(" C ,");
  lcd.setCursor(0, 1);
  lcd.print(waterlevel * 2.54); 
  lcd.print(" cm, ");
  
  delay(DELAY);
}
