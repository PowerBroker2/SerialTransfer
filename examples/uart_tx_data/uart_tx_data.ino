#include "SerialTransfer.h"


SerialTransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[] = "hello";


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);
	myTransfer.begin(Serial1, Serial);

	testStruct.z = '$';
	testStruct.y = 4.5;
}


void loop()
{
	///////////////////////////////////////// Stuff buffer with struct
	myTransfer.txObj(testStruct);

	///////////////////////////////////////// Stuff buffer with array
	myTransfer.txObj(arr);

	///////////////////////////////////////// Send buffer
	myTransfer.sendPacket();
	delay(500);
}
