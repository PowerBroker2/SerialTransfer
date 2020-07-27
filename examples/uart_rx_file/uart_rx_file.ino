#include "SerialTransfer.h"


SerialTransfer myTransfer;

const int fileSize = 2000;
char      file[fileSize];
char      fileName[10];


void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	myTransfer.begin(Serial1, Serial);
}


void loop()
{
	if (myTransfer.available())
	{
		if (myTransfer.getPacketID() == 0)
		{
			myTransfer.rxObj(fileName);
			Serial.println();
			Serial.println(fileName);
		}
		else if (myTransfer.getPacketID() == 1)
		{
			Serial.write(myTransfer.rxBuff + 2, myTransfer.getPacketSize() - 2);
		}

		Serial.println();
	}
}
