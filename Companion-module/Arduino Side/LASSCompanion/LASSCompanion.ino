#define BOARD_AVR

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include "images.c"
Adafruit_SSD1306 display(99); //Never used if you use 4pin OLED_LCD

#define PINOUT 13 


//###somthing about LASS###
//const char topic[]="LASS/Test/PM25";
//String msg="|ver_format=3|fmt_opt=0|app=PM25|ver_app=0.7.13|device_id=FT1_978|tick=262551421|date=2016-01-14|time=06:26:48|device=LinkItONE|s_0=4393.00|s_1=100.00|s_2=1.00|s_3=0.00|s_4=7.00|s_d0=42.00|s_t0=24.50|s_h0=57.20|s_d1=57.00|gps_lat=22.372586|gps_lon=120.188243|gps_fix=1|gps_num=16|gps_alt=1";
String msg;
const char seperator='|';
const char assigner='=';
//#########################

//TAGS MATCHING
#define data_num 6
#ifdef BOARD_AVR //like your 7688 Duo is AVR inside....also UNO MEGA LERONARDO bra bra bra....you can use internal memory to reduce mem usage...
  #include <avr/pgmspace.h>
  const char data_datep[] PROGMEM = "date";
  const char data_timep[] PROGMEM = "time";
  const char interestValue_1[] PROGMEM = "s_d0";
  const char interestValue_2[] PROGMEM = "s_t0";
  const char interestValue_3[] PROGMEM = "s_h0";
  const char interestValue_4[] PROGMEM = "s_d1";
  const char* const interestValue[] PROGMEM = {data_datep, data_timep , interestValue_1, interestValue_2, interestValue_3, interestValue_4, };
#else  //if something like LinkitOne .... Then .... you know it's Deluxe....
  const char data_datep[] PROGMEM = "date";
  const char data_timep[] PROGMEM = "time";
  const char interestValue_1[]  = "s_d0";
  const char interestValue_2[]  = "s_t0";
  const char interestValue_3[]  = "s_h0";
  const char interestValue_4[]  = "s_d1";
  const char* const interestValue[]  = {data_datep, data_timep ,interestValue_1, interestValue_2, interestValue_3, interestValue_4};
#endif

String data_date;
String data_time;
float sensorValue[4];
char buffer[8];

boolean onMessage(){
  Serial.println("OnMessage");
  for(int i=0;i<=data_num-1;i++){
    #ifdef BOARD_AVR
      strcpy_P(buffer, (char*)pgm_read_word(&(interestValue[i])));
    #else
      strcpy(buffer,interestValue[i]);
    #endif
   byte startpos=msg.indexOf(buffer);
   String temp =msg.substring(msg.indexOf(assigner,startpos)+1,msg.indexOf(seperator,startpos)); //From tagname after'=' to seperator'|'
   if(temp==""){
    return 0; //if failed to match tags....
    }
   if(i==0){
    data_date = temp; 
   } else if(i==1){
    data_time = temp;
   } else {
    sensorValue[i-2]=temp.toFloat();
   }
    Serial.println(temp);
  }
  return 1; // I count 2440 message/second with static process , so maybe perfomence is about 100~1000 message/s.
}

void bootuplogo(){
  char shift=0;
  while(millis() < 10000){
    display.clearDisplay();   // clears the screen and buffer
    display.drawBitmap(0, shift,  lasslogo, 128, 64, 1);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(80,0);
    display.print("Boot:");
    display.print(millis()/1000);
    display.display();
  }
  boolean direction=0;
  while(millis() < 35000){
    display.clearDisplay();   // clears the screen and buffer
    display.drawBitmap(0, shift,  lasslogo, 128, 64, 1);
    display.drawBitmap(0, shift+64,  rododo, 128, 64, 1);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(80,0);
    display.print("Boot:");
    display.print(millis()/1000);
    if(!direction){
    if(shift>=-56) { 
      shift-=8;
      } else {
        if(millis() > 20000){
          direction = 1;
        }
      }
    } else{
      if(shift<=-8) { 
      shift+=8;      
      } 
    }
    if(millis() >30000){
      direction = 0;
    }
    display.display();
    if(Serial1.available() > 0) break;
  }
}

void showLCD(byte page)
{
  display.clearDisplay();
  display.drawBitmap(0, 0,  cbartop, 128, 16, 1);

  if(page ==0){
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(6,16);
      display.print("PM2.5: ");
      display.print(sensorValue[0],0);
      display.print("ug/m3");
      display.drawBitmap(0, 24,  ctemp, 32, 16, 1);
      display.setCursor(26,28);
      display.print(":");
      display.print(sensorValue[1],1);
      display.print(" C");
      display.drawBitmap(64, 24,  chumd, 32, 16, 1);
      display.setCursor(90,28);
      display.print(": ");
      display.print(sensorValue[2],0);
      display.print(" %");
      display.drawBitmap(0, 44,  ctime, 32, 16, 1);
      display.setCursor(26,48);
      display.print(":");
      display.setCursor(40,44);
      display.print(data_date);
      display.setCursor(40,53);
      display.print(data_time);
  } else if(page ==1){
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(6,16);
      display.print(sensorValue[0],0);
      display.print(" ug/m3");
      if(sensorValue[0] <= 12){
        display.drawBitmap(16, 32,  air_0, 96, 32, 1);
      } else if(sensorValue[0] >= 12 && sensorValue[0] <= 35) {
        display.drawBitmap(16, 32,  air_1, 96, 32, 1);
      } else if(sensorValue[0] > 35 && sensorValue[0] <= 55) {
        display.drawBitmap(16, 32,  air_2, 96, 32, 1);
      } else if(sensorValue[0] > 55 && sensorValue[0] <= 150) {
        display.drawBitmap(16, 32,  air_3, 96, 32, 1);
      } else if(sensorValue[0] > 150 && sensorValue[0] <= 250) {
        display.drawBitmap(16, 32,  air_4, 96, 32, 1);
      } else if(sensorValue[0] > 250) {
        display.drawBitmap(16, 32,  air_5, 96, 32, 1);
      }
      
    }
    display.display();
}

unsigned int lcdflashtime(int *lookup){
  switch(*lookup){
    case 0:
      return 1500;
    case 1:
      return 3000;
  }
}


#define pages 2
int page=0;
unsigned long currenttime=0;
void lcdloop() {
  if((millis() - currenttime) > lcdflashtime(&page)){
    currenttime = millis();
    showLCD(page);
    page=(page+1) % pages;
  }
}

int n;
boolean logichandle = 0;
long entertime;
void messageloop(){
    while (Serial1.available() > 0){
    char tempc=Serial1.read();
    msg.concat(tempc);
    if (msg.endsWith("\n")){
      Serial.println(msg);
      if(onMessage()){
        logichandle =1;
      }
      msg="";
      n=0;
      break;
    } else {
      n++;
      if(n==350){
        Serial.println(msg);
        Serial.println(F("[ERROR] Buffer Over Flow"));
        n=0;
        msg=""; 
        break;
      }
    }
    entertime = millis();
  }
}



void setup() {
  msg="";
  msg.reserve(350);
// put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(57600);
  delay(100);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize OLED LCD
//Bootup wifi takes 35 sec. play LOGO.
  
  pinMode(OUTPUT,PINOUT);
  bootuplogo();
}

long lastOnTime;
long heartbeat;
#define ONvalue 55
#define OFFvalue 40
#define LEASTONTIME 300000
#define MOSTONTIMME 3000000

void loop() {
  // put your main code here, to run repeatedly:
  messageloop();
  long nowtime = millis();
  //prevent LCD's library delay
  if((nowtime - entertime) >100){
    lcdloop(); 
  }

 if(logichandle){
  //Run your logic here;example give you standard windows control
  if(sensorValue[0] > ONvalue){
    digitalWrite(PINOUT,HIGH); //Turn ON
    lastOnTime =nowtime;
  }

  if(sensorValue[0] <OFFvalue && (nowtime - lastOnTime) > LEASTONTIME){
    digitalWrite(PINOUT,LOW); //Turn OFF
  }
  heartbeat =nowtime;
  //after done place logic handle 0;
  logichandle=0;  
 }

 if((nowtime - heartbeat) >MOSTONTIMME){
  digitalWrite(PINOUT,LOW); //Turn OFF Because no connection;
 }
}
