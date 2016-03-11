#include <ArduinoJson.h>
#include <SoftwareSerial.h>
SoftwareSerial Serial1(0, 1);



char HistoryData[] = "{\"device_id\":\"FT1_032\",\"feeds\":[{\"timestamp\":\"2016-03-09T00:00:00Z\",\"temperature\":25.52,\"humidity\":87.79,\"PM2_5\":35.36,\"PM10\":44.18},{\"timestamp\":\"2016-03-09T01:00:00Z\",\"temperature\":25.34,\"humidity\":89.72,\"PM2_5\":47.93,\"PM10\":61.12},{\"timestamp\":\"2016-03-09T02:00:00Z\",\"temperature\":25.31,\"humidity\":89.14,\"PM2_5\":52.28,\"PM10\":66.95},{\"timestamp\":\"2016-03-09T03:00:00Z\",\"temperature\":25.39,\"humidity\":87.49,\"PM2_5\":50.18,\"PM10\":64.28},{\"timestamp\":\"2016-03-09T04:00:00Z\",\"temperature\":25.7,\"humidity\":84.76,\"PM2_5\":47.42,\"PM10\":60.85},{\"timestamp\":\"2016-03-09T05:00:00Z\",\"temperature\":24.86,\"humidity\":77.14,\"PM2_5\":39.44,\"PM10\":50.49},{\"timestamp\":\"2016-03-09T06:00:00Z\",\"temperature\":24.22,\"humidity\":73.35,\"PM2_5\":39.7,\"PM10\":50.82},{\"timestamp\":\"2016-03-09T07:00:00Z\",\"temperature\":23.79,\"humidity\":71.51,\"PM2_5\":40.2,\"PM10\":51.08},{\"timestamp\":\"2016-03-09T08:00:00Z\",\"temperature\":23.61,\"humidity\":72.35,\"PM2_5\":39.22,\"PM10\":49.05},{\"timestamp\":\"2016-03-09T09:00:00Z\",\"temperature\":23.25,\"humidity\":68.96,\"PM2_5\":34.11,\"PM10\":41.21},{\"timestamp\":\"2016-03-09T10:00:00Z\",\"temperature\":23.03,\"humidity\":69.02,\"PM2_5\":36.38,\"PM10\":44.72},{\"timestamp\":\"2016-03-09T11:00:00Z\",\"temperature\":23.31,\"humidity\":68.6,\"PM2_5\":36.13,\"PM10\":44.77},{\"timestamp\":\"2016-03-09T12:00:00Z\",\"temperature\":23.59,\"humidity\":72.91,\"PM2_5\":31.93,\"PM10\":37.93},{\"timestamp\":\"2016-03-09T13:00:00Z\",\"temperature\":23.69,\"humidity\":76.78,\"PM2_5\":28.35,\"PM10\":33.72},{\"timestamp\":\"2016-03-09T14:00:00Z\",\"temperature\":23.55,\"humidity\":77.25,\"PM2_5\":27.3,\"PM10\":32.69},{\"timestamp\":\"2016-03-09T15:00:00Z\",\"temperature\":23.38,\"humidity\":77.5,\"PM2_5\":28.02,\"PM10\":33.45},{\"timestamp\":\"2016-03-09T16:00:00Z\",\"temperature\":23.23,\"humidity\":76.8,\"PM2_5\":25.18,\"PM10\":30.57},{\"timestamp\":\"2016-03-09T17:00:00Z\",\"temperature\":23.1,\"humidity\":76.43,\"PM2_5\":25.4,\"PM10\":30.83},{\"timestamp\":\"2016-03-09T18:00:00Z\",\"temperature\":23.11,\"humidity\":75.34,\"PM2_5\":22.18,\"PM10\":27.4},{\"timestamp\":\"2016-03-09T19:00:00Z\",\"temperature\":23.04,\"humidity\":74.49,\"PM2_5\":16,\"PM10\":21.21},{\"timestamp\":\"2016-03-09T20:00:00Z\",\"temperature\":22.88,\"humidity\":73.45,\"PM2_5\":14.87,\"PM10\":20.03},{\"timestamp\":\"2016-03-09T21:00:00Z\",\"temperature\":22.75,\"humidity\":72.36,\"PM2_5\":18,\"PM10\":22.85},{\"timestamp\":\"2016-03-09T22:00:00Z\",\"temperature\":22.68,\"humidity\":72.03,\"PM2_5\":15.73,\"PM10\":20.75},{\"timestamp\":\"2016-03-09T23:00:00Z\",\"temperature\":22.57,\"humidity\":71.94,\"PM2_5\":15.32,\"PM10\":20.12},{\"timestamp\":\"2016-03-10T00:00:00Z\",\"temperature\":22.43,\"humidity\":71.48,\"PM2_5\":11.9,\"PM10\":16.39},{\"timestamp\":\"2016-03-10T01:00:00Z\",\"temperature\":22.32,\"humidity\":70.78,\"PM2_5\":10.62,\"PM10\":14.98},{\"timestamp\":\"2016-03-10T02:00:00Z\",\"temperature\":22.23,\"humidity\":70.67,\"PM2_5\":10.32,\"PM10\":14.22},{\"timestamp\":\"2016-03-10T03:00:00Z\",\"temperature\":22.02,\"humidity\":71.28,\"PM2_5\":10.8,\"PM10\":15.37},{\"timestamp\":\"2016-03-10T04:00:00Z\",\"temperature\":21.96,\"humidity\":73.01,\"PM2_5\":12.77,\"PM10\":17.74},{\"timestamp\":\"2016-03-10T05:00:00Z\",\"temperature\":21.87,\"humidity\":73.25,\"PM2_5\":13.48,\"PM10\":18.35},{\"timestamp\":\"2016-03-10T06:00:00Z\",\"temperature\":21.66,\"humidity\":72.77,\"PM2_5\":15.97,\"PM10\":20.59}]}";
byte c_data[31];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  StaticJsonBuffer<4096> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(HistoryData);

  // Test if parsing succeeds.                                                                               
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  int i=0;
  while(i<31){
    byte sensor =root["feeds"][i]["PM2_5"];
    c_data[i] = sensor;
    Serial.println(sensor);
    i++;
  }
  //drawChart(c_data);
}

void loop() {
  // put your main code here, to run repeatedly:

}

#define Xstart 10
#define barWidth 10
void drawChart(byte* array){
  Serial.println("Draw chart");
  WriteHMIcmd("cls BLACK");
  int count=0;
  while(count<sizeof(array)){
    Serial.println(array[count]);
    char cmd[30];
    char color[10]="GREEN";
    
    if(array[count] > 12){
      memset(color,0,10);
      strcpy(color,"YELLOW");  
    } else if(array[count] > 35) {
      memset(color,0,10);
      strcpy(color,"64487"); 
    } else if(array[count] > 55) {
      memset(color,0,10);
      strcpy(color,"RED"); 
    } else if(array[count] > 151) {
      memset(color,0,10);
      strcpy(color,"30751"); 
    }
    
    sprintf(cmd,"fill %d,%d,%d,%d,%s",Xstart+count*barWidth,240-array[count]*2,barWidth,array[count]*2,color);
    WriteHMIcmd(cmd);
    sprintf(cmd,"line %d,%d,%d,%d,BLACK",Xstart+count*barWidth+barWidth-1,240,Xstart+count*barWidth+barWidth-1,240-array[count]*2);
    WriteHMIcmd(cmd);
    count++;
    }  
}

void WriteHMIcmd(String CMD){
  Serial1.print(CMD);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
}
