#include "SPITransfer.h"


SPITransfer myTransfer;

struct STRUCT
{
	char  z;
	float y;
} testStruct;

char arr[6];


void setup()
{
	Serial.begin(115200);

	pinMode(MISO, OUTPUT);

	myTransfer.begin(SPI, Serial);
}


void loop()
{
	myTransfer.rxObj(testStruct);
	Serial.print(testStruct.z);
	Serial.print(testStruct.y);
	Serial.print(" | ");

	myTransfer.rxObj(arr);
	Serial.println(arr);
}
