#include <SoftwareSerial.h>

SoftwareSerial sensor(7,8);  //pinTx, pinRx


class deviceMHZ16 {  
  #define DEBUG 0
  unsigned char dataRevice[9];
  int CO2PPM;
  int temperature; 
  const unsigned char cmd_get_sensor[9] =
      {
          0xff, 0x01, 0x86, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x79
      };
  
  public:
    deviceMHZ16() {
      sensor.begin(9600); 
      
    }

    long readValue() {
        while(!dataCO2Recieve())
        {
            delay(500);
        }
        return CO2PPM;
    }

    bool dataCO2Recieve(void)
    {
        byte data[9];
        int i = 0;
     
        //transmit command data
        for(i=0; i<sizeof(cmd_get_sensor); i++)
        {
            sensor.write(cmd_get_sensor[i]);
        }
        delay(10);
        //begin reveiceing data
        if(sensor.available())
        {
            while(sensor.available())
            {
                for(int i=0;i<9; i++)
                {
                    data[i] = sensor.read();
                }
            }
        }
     
    #if DEBUG
        for(int j=0; j<9; j++)
        {
            Serial.print(data[j]);
            Serial.print(" ");
        }
        Serial.println("");
    #endif
     
        if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3]
        + data[4] + data[5] + data[6] + data[7]))) != data[8])
        {
            return false;
        }
        CO2PPM = (int)data[2] * 256 + (int)data[3];
        temperature = (int)data[4] - 40;
     
        return true;
    }
};

deviceMHZ16 sensorMHZ16;

void setup() {
  Serial.begin(115200);
  Serial.println("MH-Z16 test!");
  
}

void loop() {
  Serial.println(sensorMHZ16.readValue());
  delay(2000);
    
}  
