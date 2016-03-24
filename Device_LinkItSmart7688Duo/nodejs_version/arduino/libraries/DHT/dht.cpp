//
//    FILE: dht22.cpp
// VERSION: 0.1.00
// PURPOSE: DHT22 Temperature & Humidity Sensor library for Arduino
//
// DATASHEET: 
//
// HISTORY:
// 0.1.0 by Rob Tillaart (01/04/2011)
// inspired by DHT11 library
//

#include "dht.h"

#define TIMEOUT 10000

/////////////////////////////////////////////////////
//
// PUBLIC
//


// return values:
//  0 : OK
// -1 : checksum error
// -2 : timeout
int dht::read11(uint8_t pin)
{
	// READ VALUES
	int rv = read(pin);
	if (rv != 0) return rv;

	// CONVERT AND STORE
	humidity    = bits[0];  // bit[1] == 0;
	temperature = bits[2];  // bits[3] == 0;

	// TEST CHECKSUM
	uint8_t sum = bits[0] + bits[2]; // bits[1] && bits[3] both 0
	if (bits[4] != sum) return -1;

	return 0;
}

// return values:
//  0 : OK
// -1 : checksum error
// -2 : timeout
int dht::read22(uint8_t pin)
{
	// READ VALUES
	int rv = read(pin);
	if (rv != 0) return rv;

	// CONVERT AND STORE
	humidity    = word(bits[0], bits[1]) * 0.1;

	int sign = 1;
	if (bits[2] & 0x80) // negative temperature
	{
		bits[2] = bits[2] & 0x7F;
		sign = -1;
	}
	temperature = sign * word(bits[2], bits[3]) * 0.1;


	// TEST CHECKSUM
	uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
	if (bits[4] != sum) return -1;

	return 0;
}

/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
//  0 : OK
// -2 : timeout
int dht::read(uint8_t pin)
{
	// INIT BUFFERVAR TO RECEIVE DATA
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	for (int i=0; i< 5; i++) bits[i] = 0;

	// REQUEST SAMPLE
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delay(20);
	digitalWrite(pin, HIGH);
	delayMicroseconds(40);
	pinMode(pin, INPUT);

	// GET ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = TIMEOUT;
	while(digitalRead(pin) == LOW)
		if (loopCnt-- == 0) return -2;

	loopCnt = TIMEOUT;
	while(digitalRead(pin) == HIGH)
		if (loopCnt-- == 0) return -2;

	// READ THE OUTPUT - 40 BITS => 5 BYTES
	for (int i=0; i<40; i++)
	{
		loopCnt = TIMEOUT;
		while(digitalRead(pin) == LOW)
			if (loopCnt-- == 0) return -2;

		unsigned long t = micros();

		loopCnt = TIMEOUT;
		while(digitalRead(pin) == HIGH)
			if (loopCnt-- == 0) return -2;

		if ((micros() - t) > 40) bits[idx] |= (1 << cnt);
		if (cnt == 0)   // next byte?
		{
			cnt = 7;   
			idx++;      
		}
		else cnt--;
	}

	return 0;
}
//
// END OF FILE
//
