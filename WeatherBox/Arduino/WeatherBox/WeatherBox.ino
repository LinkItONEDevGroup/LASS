/*
Board:
    Arduino Mega 2560
Pins:
    Digital 4: DHT22
    Digital 7: LED
    Serial 1: APRS Weather Station
    Serial 2: PlanTower G3
    Serial 3: Grove GPS
    SCL, SDA : GY-30, Si1145
*/

#include "DHT.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
#include "Adafruit_SI1145.h"
#include "TinyGPS.h"

#define DHTPIN 4  // Digital Port
#define DHTTYPE DHT22  // DHT 22  (AM2302)
#define LEDPIN 7  // Digital Port
#define SLEEPTIME 5000  // second

DHT dht(DHTPIN, DHTTYPE);

char aprs_data_buffer[35];
double aprs_result_temp;

BH1750 lightMeter;
Adafruit_SI1145 uv = Adafruit_SI1145();

TinyGPS gps;

// Humi, Temp
float humidity_22 = 0;
float temperature_22 = 0;

// Dust
long pmcf10 = 0;
long pmcf25 = 0;
long pmcf100 = 0;
long pmat10 = 0;
long pmat25 = 0;
long pmat100 = 0;

// Weather Station
double humidity_11 = 0;
double temperature_11 = 0;
double wind_direction = 0;
double wind_speed_1_min = 0;
double wind_speed_5_mins = 0;
double rainfall_1_hr = 0;
double rainfall_24_hr = 0;
double pressure = 0;

// Light Sensor
uint16_t lux = 0;

// UV Sensor
uint16_t visible_light = 0;
uint16_t infrared_light = 0;
float UVindex = 0;

// GPS
float flat, flon;
unsigned long age;

void read_dust() {
    int count = 0;
    unsigned char c;
    unsigned char high;
    
    while (Serial2.available()) {
        c = Serial2.read();

        if((count==0 && c!=0x42) || (count==1 && c!=0x4d)) {
            Serial.println("check failed");
            break;
        }
        if(count > 15) {
            //Serial.println("complete");
            break;
        }
        else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) high = c;
        else if(count == 5) {
            pmcf10 = 256*high + c;
            /*
            Serial.print("CF=1, PM1.0=");
            Serial.print(pmcf10);
            Serial.println(" ug/m3");
            */
        }
        else if(count == 7) {
            pmcf25 = 256*high + c;
            /*
            Serial.print("CF=1, PM2.5=");
            Serial.print(pmcf25);
            Serial.println(" ug/m3");
            */
        }
        else if(count == 9) {
            pmcf100 = 256*high + c;
            /*
            Serial.print("CF=1, PM10=");
            Serial.print(pmcf100);
            Serial.println(" ug/m3");
            */
        }
        else if(count == 11) {
            pmat10 = 256*high + c;
            /*
            Serial.print("atmosphere, PM1.0=");
            Serial.print(pmat10);
            */
        }
        else if(count == 13) {
            pmat25 = 256*high + c;
            /*
            Serial.print("atmosphere, PM2.5=");
            Serial.print(pmat25);
            Serial.println(" ug/m3");
            */
        }
        else if(count == 15) {
            pmat100 = 256*high + c;
            /*
            Serial.print("atmosphere, PM10=");
            Serial.print(pmat100);
            Serial.println(" ug/m3");
            */
        }

        count++;
    }
    while(Serial2.available()) Serial2.read();
}

void read_dht() {
    humidity_22 = dht.readHumidity();  // Read temperature as Celsius
    temperature_22 = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity_22) || isnan(temperature_22)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    /*
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t\n");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C ");
    */
}

void get_aprs_buffer() {                                                                // Get weather status data
    memset(aprs_data_buffer, 0, sizeof(aprs_data_buffer));
    int index;
    for (index = 0;index < 35;index ++)
    {
        if(Serial1.available())
        {

            aprs_data_buffer[index] = Serial1.read();
            if (aprs_data_buffer[0] != 'c')
            {
                index = -1;
            }
        }
//        else
//        {
//            index --;
//        }
    }
}

int transCharToInt(char *_buffer,int _start,int _stop) {                                // char to int
    int _index;
    int result = 0;
    int num = _stop - _start + 1;
    int _temp[num];
    for (_index = _start;_index <= _stop;_index ++)
    {
        _temp[_index - _start] = _buffer[_index] - '0';
        result = 10*result + _temp[_index - _start];
    }
    return result;
}

int WindDirection() {                                                                   // Wind Direction
    return transCharToInt(aprs_data_buffer, 1, 3);
}

float WindSpeedAverage() {                                                              // air Speed (1 minute)
    aprs_result_temp = 0.44704 * transCharToInt(aprs_data_buffer, 5, 7);

    return aprs_result_temp;
}

float WindSpeedMax() {                                                                  // Max air speed (5 minutes)
    aprs_result_temp = 0.44704 * transCharToInt(aprs_data_buffer, 9, 11);

    return aprs_result_temp;
}

float Temperature() {                                                                   // Temperature ("C")
    aprs_result_temp = (transCharToInt(aprs_data_buffer,13,15) - 32.00) * 5.00 / 9.00;

    return aprs_result_temp;
}

float RainfallOneHour() {                                                               // Rainfall (1 hour)
    aprs_result_temp = transCharToInt(aprs_data_buffer,17,19) * 25.40 * 0.01;

    return aprs_result_temp;
}

float RainfallOneDay() {                                                               // Rainfall (24 hours)
    aprs_result_temp = transCharToInt(aprs_data_buffer, 21, 23) * 25.40 * 0.01;

    return aprs_result_temp;
}

int Humidity() {                                                                       // Humidity
    return transCharToInt(aprs_data_buffer, 25, 26);
}

float BarPressure() {                                                                  // Barometric Pressure 
    aprs_result_temp = transCharToInt(aprs_data_buffer, 28, 32);

    return aprs_result_temp / 10.00;
}

void read_aprs() {
    get_aprs_buffer();
    
    humidity_11 = Humidity();
    temperature_11 = Temperature();
    wind_direction = WindDirection();
    wind_speed_1_min = WindSpeedAverage();
    wind_speed_5_mins = WindSpeedMax();
    rainfall_1_hr = RainfallOneHour();
    rainfall_24_hr = RainfallOneDay();
    pressure = BarPressure();

    /*
    Serial.print("Wind Direction: ");
    Serial.print(wind_direction);
    Serial.println("  ");
    Serial.print("Average Wind Speed (One Minute): ");
    Serial.print(wind_speed_1_min);
    Serial.println("m/s  ");
    Serial.print("Max Wind Speed (Five Minutes): ");
    Serial.print(wind_speed_5_mins);
    Serial.println("m/s");
    Serial.print("Rain Fall (One Hour): ");
    Serial.print(rainfall_1_hr);
    Serial.println("mm  ");
    Serial.print("Rain Fall (24 Hour): ");
    Serial.print(rainfall_24_hr);
    Serial.println("mm");
    Serial.print("Temperature: ");
    Serial.print(temperature_11);
    Serial.println("C  ");
    Serial.print("Humidity: ");
    Serial.print(humidity_11);
    Serial.println("%  ");
    Serial.print("Barometric Pressure: ");
    Serial.print(pressure);
    Serial.println("hPa");
    Serial.println("");
    Serial.println("");
    */
}

void read_lux() {
    lux = lightMeter.readLightLevel();
    /*
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    */
}

void read_uv() {
    visible_light = uv.readVisible();
    infrared_light = uv.readIR();
    UVindex = uv.readIR() / 100.0;  
    /*
    Serial.print("Vis: "); Serial.println(visible_light);
    Serial.print("IR: "); Serial.println(infrared_light);
    Serial.print("UV: ");  Serial.println(UVindex);
    */
}

void read_gps() {
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial3.available())
    {
      char c = Serial3.read();
      // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    gps.f_get_position(&flat, &flon, &age);
    /*
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    */
  }
  
  gps.stats(&chars, &sentences, &failed);
  /*
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  */
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
}

void setup() {
    Serial.begin(9600);  // Rsapberry Pi
    Serial1.begin(9600);  // APRS Weather Station
    Serial2.begin(9600);  // G3
    Serial3.begin(9600);  // Grove GPS
    dht.begin();
    lightMeter.begin();
    uv.begin();
    pinMode(LEDPIN, OUTPUT);
}

void loop() {
    StaticJsonBuffer<1536> jsonBuffer;
    JsonArray& json_array = jsonBuffer.createArray();

    StaticJsonBuffer<192> json_dht22_buffer;
    StaticJsonBuffer<128> json_dht22_sensors_buffer;
    StaticJsonBuffer<64> json_dht22_humidity_buffer;
    StaticJsonBuffer<64> json_dht22_temperature_buffer;
    JsonObject& json_dht22 = json_dht22_buffer.createObject();
    JsonArray& json_dht22_sensors = json_dht22_sensors_buffer.createArray();
    JsonObject& json_dht22_humidity = json_dht22_humidity_buffer.createObject();
    JsonObject& json_dht22_temperature = json_dht22_temperature_buffer.createObject();

    StaticJsonBuffer<448> json_g3_buffer;
    StaticJsonBuffer<384> json_g3_sensors_buffer;
    StaticJsonBuffer<64> json_g3_pmcf10_buffer;
    StaticJsonBuffer<64> json_g3_pmcf25_buffer;
    StaticJsonBuffer<64> json_g3_pmcf100_buffer;
    StaticJsonBuffer<64> json_g3_pmat10_buffer;
    StaticJsonBuffer<64> json_g3_pmat25_buffer;
    StaticJsonBuffer<64> json_g3_pmat100_buffer;
    JsonObject& json_g3 = json_g3_buffer.createObject();
    JsonArray& json_g3_sensors = json_g3_sensors_buffer.createArray();
    JsonObject& json_g3_pmcf10 = json_g3_pmcf10_buffer.createObject();
    JsonObject& json_g3_pmcf25 = json_g3_pmcf25_buffer.createObject();
    JsonObject& json_g3_pmcf100 = json_g3_pmcf100_buffer.createObject();
    JsonObject& json_g3_pmat10 = json_g3_pmat10_buffer.createObject();
    JsonObject& json_g3_pmat25 = json_g3_pmat25_buffer.createObject();
    JsonObject& json_g3_pmat100 = json_g3_pmat100_buffer.createObject();

    StaticJsonBuffer<576> json_aprs_buffer;
    StaticJsonBuffer<512> json_aprs_sensors_buffer;
    StaticJsonBuffer<64> json_aprs_humidity_buffer;
    StaticJsonBuffer<64> json_aprs_temperature_buffer;
    StaticJsonBuffer<64> json_aprs_wind_direction_buffer;
    StaticJsonBuffer<64> json_aprs_wind_speed_1_min_buffer;
    StaticJsonBuffer<64> json_aprs_wind_speed_5_mins_buffer;
    StaticJsonBuffer<64> json_aprs_rainfall_1_hr_buffer;
    StaticJsonBuffer<64> json_aprs_rainfall_24_hr_buffer;
    StaticJsonBuffer<64> json_aprs_pressure_buffer;
    JsonObject& json_aprs = json_aprs_buffer.createObject();
    JsonArray& json_aprs_sensors = json_aprs_sensors_buffer.createArray();
    JsonObject& json_aprs_humidity = json_aprs_humidity_buffer.createObject();
    JsonObject& json_aprs_temperature = json_aprs_temperature_buffer.createObject();
    JsonObject& json_aprs_wind_direction = json_aprs_wind_direction_buffer.createObject();
    JsonObject& json_aprs_wind_speed_1_min = json_aprs_wind_speed_1_min_buffer.createObject();
    JsonObject& json_aprs_wind_speed_5_mins = json_aprs_wind_speed_5_mins_buffer.createObject();
    JsonObject& json_aprs_rainfall_1_hr = json_aprs_rainfall_1_hr_buffer.createObject();
    JsonObject& json_aprs_rainfall_24_hr = json_aprs_rainfall_24_hr_buffer.createObject();
    JsonObject& json_aprs_pressure = json_aprs_pressure_buffer.createObject();

    StaticJsonBuffer<128> json_gy30_buffer;
    StaticJsonBuffer<64> json_gy30_sensors_buffer;
    StaticJsonBuffer<64> json_gy30_lux_buffer;
    JsonObject& json_gy30 = json_gy30_buffer.createObject();
    JsonArray& json_gy30_sensors = json_gy30_sensors_buffer.createArray();
    JsonObject& json_gy30_lux = json_gy30_lux_buffer.createObject();

    StaticJsonBuffer<192> json_si1145_buffer;
    StaticJsonBuffer<128> json_si1145_sensors_buffer;
    StaticJsonBuffer<64> json_si1145_visiable_light_buffer;
    StaticJsonBuffer<64> json_si1145_uvindex_buffer;
    JsonObject& json_si1145 = json_si1145_buffer.createObject();
    JsonArray& json_si1145_sensors = json_si1145_sensors_buffer.createArray();
    JsonObject& json_si1145_visiable_light = json_si1145_visiable_light_buffer.createObject();
    JsonObject& json_si1145_uvindex = json_si1145_uvindex_buffer.createObject();

    StaticJsonBuffer<192> json_grove_gps_buffer;
    StaticJsonBuffer<128> json_grove_gps_sensors_buffer;
    StaticJsonBuffer<64> json_grove_gps_lat_buffer;
    StaticJsonBuffer<64> json_grove_gps_lon_buffer;
    JsonObject& json_grove_gps = json_grove_gps_buffer.createObject();
    JsonArray& json_grove_gps_sensors = json_grove_gps_sensors_buffer.createArray();
    JsonObject& json_grove_gps_lat = json_grove_gps_lat_buffer.createObject();
    JsonObject& json_grove_gps_lon = json_grove_gps_lon_buffer.createObject();
    
    /* sensors read start */
    digitalWrite(LEDPIN, LOW);  // power on the LED
    read_dht();
    read_dust();
    read_aprs();
    read_lux();
    read_uv();
    read_gps();
    digitalWrite(LEDPIN, HIGH); // turn the LED off
    /* sensors read end */

    /* encoding JSON start */
    json_dht22["brand"] = "Adafruit";
    json_dht22["module"] = "DHT22";
    json_dht22_humidity["name"] = "Humidity";
    json_dht22_humidity["value"] = humidity_22;
    json_dht22_humidity["unit"] = "%";
    json_dht22_temperature["name"] = "Temperature";
    json_dht22_temperature["value"] = temperature_22;
    json_dht22_temperature["unit"] = "°C";
    json_dht22_sensors.add(json_dht22_humidity);
    json_dht22_sensors.add(json_dht22_temperature);
    json_dht22["sensors"] = json_dht22_sensors;
    json_array.add(json_dht22);
    
    json_g3["brand"] = "Plantower";
    json_g3["module"] = "G3";
    json_g3_pmcf10["name"] = "PM 1.0 (TSI standard)";
    json_g3_pmcf10["value"] = pmcf10;
    json_g3_pmcf10["unit"] = "ug/m3";
    json_g3_pmcf25["name"] = "PM 2.5 (TSI standard)";
    json_g3_pmcf25["value"] = pmcf25;
    json_g3_pmcf25["unit"] = "ug/m3";
    json_g3_pmcf100["name"] = "PM 10 (TSI standard)";
    json_g3_pmcf100["value"] = pmcf100;
    json_g3_pmcf100["unit"] = "ug/m3";
    json_g3_pmat10["name"] = "PM 1.0 (std. atmosphere)";
    json_g3_pmat10["value"] = pmat10;
    json_g3_pmat10["unit"] = "ug/m3";
    json_g3_pmat25["name"] = "PM 2.5 (std. atmosphere)";
    json_g3_pmat25["value"] = pmat25;
    json_g3_pmat25["unit"] = "ug/m3";
    json_g3_pmat100["name"] = "PM 10 (std. atmosphere)";
    json_g3_pmat100["value"] = pmat100;
    json_g3_pmat100["unit"] = "ug/m3";
    json_g3_sensors.add(json_g3_pmcf10);
    json_g3_sensors.add(json_g3_pmcf25);
    json_g3_sensors.add(json_g3_pmcf100);
    json_g3_sensors.add(json_g3_pmat10);
    json_g3_sensors.add(json_g3_pmat25);
    json_g3_sensors.add(json_g3_pmat100);
    json_g3["sensors"] = json_g3_sensors;
    json_array.add(json_g3);

    json_aprs["brand"] = "DFRobot";
    json_aprs["module"] = "APRS Weather Station";
    json_aprs_humidity["name"] = "Humidity";
    json_aprs_humidity["value"] = humidity_11;
    json_aprs_humidity["unit"] = "%";
    json_aprs_temperature["name"] = "Temperature";
    json_aprs_temperature["value"] = temperature_11;
    json_aprs_temperature["unit"] = "°C";
    json_aprs_wind_direction["name"] = "Wind Direction";
    json_aprs_wind_direction["value"] = wind_direction;
    json_aprs_wind_direction["unit"] = "°";
    json_aprs_wind_speed_1_min["name"] = "Average Wind Speed (1 Minute)";
    json_aprs_wind_speed_1_min["value"] = wind_speed_1_min;
    json_aprs_wind_speed_1_min["unit"] = "m/s";
    json_aprs_wind_speed_5_mins["name"] = "Max Wind Speed (5 Minutes)";
    json_aprs_wind_speed_5_mins["value"] = wind_speed_5_mins;
    json_aprs_wind_speed_5_mins["unit"] = "m/s";
    json_aprs_rainfall_1_hr["name"] = "Rainfall (1 Hour)";
    json_aprs_rainfall_1_hr["value"] = rainfall_1_hr;
    json_aprs_rainfall_1_hr["unit"] = "mm";
    json_aprs_rainfall_24_hr["name"] = "Rainfall (24 Hour)";
    json_aprs_rainfall_24_hr["value"] = rainfall_24_hr;
    json_aprs_rainfall_24_hr["unit"] = "mm";
    json_aprs_pressure["name"] = "Atmosphere Pressure";
    json_aprs_pressure["value"] = pressure;
    json_aprs_pressure["unit"] = "hpa";
    json_aprs_sensors.add(json_aprs_humidity);
    json_aprs_sensors.add(json_aprs_temperature);
    json_aprs_sensors.add(json_aprs_wind_direction);
    json_aprs_sensors.add(json_aprs_wind_speed_1_min);
    json_aprs_sensors.add(json_aprs_wind_speed_5_mins);
    json_aprs_sensors.add(json_aprs_rainfall_1_hr);
    json_aprs_sensors.add(json_aprs_rainfall_24_hr);
    json_aprs_sensors.add(json_aprs_pressure);
    json_aprs["sensors"] = json_aprs_sensors;
    json_array.add(json_aprs);

    json_gy30["brand"] = "Adafruit";
    json_gy30["module"] = "GY-30";
    json_gy30_lux["name"] = "Lux";
    json_gy30_lux["value"] = lux;
    json_gy30_lux["unit"] = "lx";
    json_gy30_sensors.add(json_gy30_lux);
    json_gy30["sensors"] = json_gy30_sensors;
    json_array.add(json_gy30);
    
    json_si1145["brand"] = "Adafruit";
    json_si1145["module"] = "Si1145";
    json_si1145_visiable_light["name"] = "Visiable Light";
    json_si1145_visiable_light["value"] = visible_light;
    json_si1145_visiable_light["unit"] = "nm";
    json_si1145_uvindex["name"] = "UV Index";
    json_si1145_uvindex["value"] = UVindex;
    json_si1145_uvindex["unit"] = "";
    json_si1145_sensors.add(json_si1145_visiable_light);
    json_si1145_sensors.add(json_si1145_uvindex);
    json_si1145["sensors"] = json_si1145_sensors;
    json_array.add(json_si1145);

    json_grove_gps["brand"] = "Grove";
    json_grove_gps["module"] = "GPS";
    json_grove_gps_lat["name"] = "Lat";
    json_grove_gps_lat["value"].set(flat, 6);
    json_grove_gps_lat["unit"] = "";
    json_grove_gps_lon["name"] = "Lon";
    json_grove_gps_lon["value"].set(flon, 6);
    json_grove_gps_lon["unit"] = "";
    json_grove_gps_sensors.add(json_grove_gps_lat);
    json_grove_gps_sensors.add(json_grove_gps_lon);
    json_grove_gps["sensors"] = json_grove_gps_sensors;
    json_array.add(json_grove_gps);
    /* encoding JSON end */

    json_array.printTo(Serial);
    Serial.println();

    delay(SLEEPTIME);
}
