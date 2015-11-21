/*macro definition of water sensor and the buzzer*/
//STATUS: not confirmed work. debugging.
#define WATER_SENSOR 7
void setup()
{
	pins_init();
        Serial.begin(115200); 
}
void loop()
{
	if(isExposedToWater()){
          Serial.println("Water In");
        }else{
          Serial.println("Water out");
        }  
        delay(1000);
}
void pins_init()
{
	pinMode(WATER_SENSOR, INPUT);
}
/************************************************************************/
/*Function: Determine whether the sensor is exposed to the water		*/
/*Parameter:-void           											*/
/*Return:	-boolean,if it is exposed to the water,it will return true. */
boolean isExposedToWater()
{
	if(digitalRead(WATER_SENSOR) == LOW)
		return true;
	else return false;
}
