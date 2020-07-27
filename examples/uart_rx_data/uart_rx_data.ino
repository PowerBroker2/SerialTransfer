#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[6];


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	// Serial is the debug serial port. If you don't want to enable debugging, you can remove it
	myTransfer.begin(Serial1, Serial);
}


void loop()
{
	if (myTransfer.available())
	{
		myTransfer.rxObj(testStruct);
		Serial.print(testStruct.z);
		Serial.print(testStruct.y);
		Serial.print(" | ");

		myTransfer.rxObj(arr);
		Serial.println(arr);
	}
}
