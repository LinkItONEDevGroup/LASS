//////////////////////////////////////////////////
// Radiation-Watch.org
// URL http://www.radiation-watch.org/
//////////////////////////////////////////////////

///　Digital I/O PIN Settings　///
int signPin = 2; //Radiation Pulse (Yellow)
int noisePin = 5; //Vibration Noise Pulse (White)
//VCC 5V (Red)
//GND (Blue)
////////////////////////////////

const double alpha=53.032; // cpm = uSv x alpha

int count=0; //Number of loops
char msg[256]=""; //Message buffer for serial output

int signCount=0;  //Counter for Radiation Pulse
int noiseCount=0;  //Counter for Noise Pulse

int sON=0;//Lock flag for Radiation Pulse
int nON=0;//Lock flag for Noise Puls

double cpm = 0; //Count rate [cpm] of current
double cpmHistory[200]; //History of count rates
int cpmIndex=0;//Position of current count rate on cpmHistory[]
int cpmIndexPrev=0;//Flag to prevent duplicative counting

//Timing Settings for Loop Interval
int prevTime=0;
int currTime=0; 

int totalSec=0; //Elapsed time of measurement [sec]
int totalHour=0; //Elapsed time of measurement [hour]

//Time settings for CPM calcuaration
int cpmTimeMSec=0;
int cpmTimeSec=0;
int cpmTimeMin=0;

//String buffers of float values for serial output
char cpmBuff[20];
char uSvBuff[20];
char uSvdBuff[20];

void setup()
{
  //Serial setup
  //9600bps
  Serial.begin(9600);
  
  //PIN setting for Radiation Pulse
  pinMode(signPin,INPUT);
  digitalWrite(signPin,HIGH);

  //PIN setting for Noise Pulse
  pinMode(noisePin,INPUT);
  digitalWrite(noisePin,HIGH);

  //CSV-formatting for serial output (substitute , for _)
  Serial.println("hour[h]_sec[s]_count_cpm_uSv/h_uSv/hError");
  
  //Initialize cpmHistory[]
  for(int i=0; i<200;i++ )
  {
    cpmHistory[i]=0;
  }
  
  //Get start time of a loop
  prevTime = millis();
}

void loop()
{
  // Raw data of Radiation Pulse: Not-detected -> High, Detected -> Low
  int sign = digitalRead(signPin);

  // Raw data of Noise Pulse: Not-detected -> Low, Detected -> High
  int noise = digitalRead(noisePin);

  //Radiation Pulse normally keeps low for about 100[usec]
  if(sign==0 && sON==0)
  {//Deactivate Radiation Pulse counting for a while
    sON = 1;
    signCount++;
  }else if(sign==1 && sON==1){
    sON = 0;
  }

  //Noise Pulse normally keeps high for about 100[usec]
  if(noise==1 && nON==0)
  {//Deactivate Noise Pulse counting for a while
    nON = 1;
    noiseCount++;
  }else if(noise==0 && nON==1){
    nON = 0;
  }

  //Output readings to serial port, after 10000 loops
  if(count==10000) //About 160-170 msec in Arduino Nano(ATmega328)
  {
    //Get current time
    currTime = millis();
    
    //No noise detected in 10000 loops
    if(noiseCount == 0)
    {
      //Shift an array for counting log for each 6 sec.
      if( totalSec % 6 == 0 && cpmIndexPrev != totalSec)
      {
        cpmIndexPrev = totalSec;
        cpmIndex++;
        
        if(cpmIndex >= 200)
        {
          cpmIndex = 0;
        }
        
        if(cpmHistory[cpmIndex] > 0)
        {
          cpm -= cpmHistory[cpmIndex];
        }
        cpmHistory[cpmIndex]=0;
      }
      
      //Store count log
      cpmHistory[cpmIndex] += signCount;
      //Add number of counts
      cpm += signCount;
      
      //Get ready time for 10000 loops
      cpmTimeMSec += abs(currTime - prevTime);
      //Transform from msec. to sec. (to prevent overflow)
      if(cpmTimeMSec >= 1000)
      {
        cpmTimeMSec -= 1000;
        //Add measurement time to calcurate cpm readings (max=20min.)
        if( cpmTimeSec >= 20*60 )
        {
          cpmTimeSec = 20*60;
        }else{
          cpmTimeSec++;
        }
        
        //Total measurement time
        totalSec++;
        //Transform from sec. to hour. (to prevent overflow)
        if(totalSec >= 3600)
        {
          totalSec -= 3600;
          totalHour++;
        }
      }
      
      //Elapsed time of measurement (max=20min.)
      double min = cpmTimeSec / 60.0;
      if(min!=0)
      {
        //Calculate cpm, uSv/h and error of uSv/h
        sprintf(cpmBuff, "%f", cpm / min);
        sprintf(uSvBuff, "%f", cpm / min / alpha);
        sprintf(uSvdBuff, "%f", sqrt(cpm) / min / alpha);
      }else{
        //Devision by zero
        sprintf(cpmBuff, "%f", 0.0);
        sprintf(uSvBuff, "%f", 0.0);
        sprintf(uSvdBuff, "%f", 0.0);
      }
        
      //Create message for serial port
      sprintf(msg, "%d,%d.%03d,%d,%s,%s,%s",
      totalHour,totalSec,
      cpmTimeMSec,
      signCount,
      cpmBuff,
      uSvBuff,
      uSvdBuff
        );
        
      //Send message to serial port
      Serial.println(msg);
      
    }
    
    //Initialization for next 10000 loops
    prevTime = currTime;
    signCount=0;
    noiseCount=0;
    count=0;
  }
  count++;
}


