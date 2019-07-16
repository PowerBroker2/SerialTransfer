/////////////////////////////////////////////////////////////////////////////////////////
/*
   Command Array Anatomy:

   --------------------------------------------------------------------------------------
   Index Number - Command Type                    - Command Format
   --------------------------------------------------------------------------------------
		0       - pitch                           - unsigned 16-Bit Analog
		1       - roll                            - unsigned 16-Bit Analog
		2       - yaw                             - unsigned 16-Bit Analog
		3       - throttle                        - unsigned 16-Bit Analog
		4       - Autopilot Toggle                - unsigned 16-Bit  Boolean (0x0 or 0x1)
		5       - Pitch/Roll Limiter Toggle       - unsigned 16-Bit  Boolean (0x0 or 0x1)
		6       - Landing Gear Toggle             - unsigned 16-Bit  Boolean (0x0 or 0x1)
		7       - Flaps Toggle                    - unsigned 16-Bit  Boolean (0x0 or 0x1)
		8       - Unused                          - Unused
		9       - Unused                          - Unused
		10      - Unused                          - Unused
		11      - Unused                          - Unused
		12      - Unused                          - Unused
		13      - Unused                          - Unused
		14      - Unused                          - Unused
		15      - Unused                          - Unused
		16      - Unused                          - Unused
		17      - Unused                          - Unused
		18      - Unused                          - Unused
		19      - Unused                          - Unused
*/
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
/*
   Telemetry Array Anatomy:

   --------------------------------------------------------------------------------------
   Index Number - Telemetry Type                  - Telemetry Format
   --------------------------------------------------------------------------------------
		0       - Pitot Velocity (m/s) *100       - unsigned 16-bit  Analog
		1       - Converted Altitude (m) * 100    - unsigned 16-bit  Analog
		2       - Pitch Angle (degrees) * 100     - signed 16-bit  Analog
		3       - Roll Angle (degrees) * 100      - signed 16-bit  Analog
		4       - Latitude (dd) * 100             - signed 16-bit  Analog
		5       - Longitude (dd) * 100            - signed 16-bit  Analog
		6       - Unused                          - Unused
		7       - Unused                          - Unused
		8       - Unused                          - Unused
		9       - Unused                          - Unused
		10      - Unused                          - Unused
		11      - Unused                          - Unused
		12      - Unused                          - Unused
		13      - Unused                          - Unused
		14      - Unused                          - Unused
		15      - Unused                          - Unused
		16      - Unused                          - Unused
		17      - Unused                          - Unused
		18      - Unused                          - Unused
		19      - Unused                          - Unused
*/
/////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"




#define AIR_DATA_LEN            20                      //number of 16bit numbers to transfer
#define BUFF_LEN                (AIR_DATA_LEN * 2) + 3  //number of total bytes per transmission (including BUFF_LEN, checksum, and END_BYTE)
#define START_BYTE              0x7E                    //dataframe start byte
#define END_BYTE                0xEF                    //dataframe end byte

//incoming serial data/parsing errors
#define AIR_NO_DATA             0
#define AIR_NEW_DATA            1
#define AIR_CHECKSUM_ERROR      -1
#define AIR_PAYLOAD_ERROR       -2

//packet index identifiers
///////////////////////////////////////////////////////////////////////////////////////// COMMAND
#define AIR_PITCH_INDEX         0
#define AIR_ROLL_INDEX          1
#define AIR_YAW_INDEX           2
#define AIR_THROTTLE_INDEX      3
#define AIR_AUTOPILOT_INDEX     4
#define AIR_LIMITER_INDEX       5
#define AIR_LANDING_GEAR_INDEX  6
#define AIR_FLAPS_INDEX         7

///////////////////////////////////////////////////////////////////////////////////////// TELEMETRY
#define AIR_PITOT_INDEX         0
#define AIR_ALTITUDE_INDEX      1
#define AIR_PITCH_ANGLE_INDEX   2
#define AIR_ROLL_ANGLE_INDEX    3
#define AIR_LATITUDE_INDEX      4
#define AIR_LONGITUDE_INDEX     5
#define AIR_UTC_YEAR_INDEX      6
#define AIR_UTC_MONTH_INDEX     7
#define AIR_UTC_DAY_INDEX       8
#define AIR_UTC_HOUR_INDEX      9
#define AIR_UTC_MINUTE_INDEX    10
#define AIR_UTC_SECOND_INDEX    11
#define AIR_SOG_INDEX           12
#define AIR_COG_INDEX           13




class airComms
{
public:// <<---------------------------------------------------------------------------//public

	byte inputArray_Radio[BUFF_LEN] = { 0 };	//an array to hold incoming data
	byte inputArray_CurrentIndex = 0;			//index to see where in the input array we are at
	bool arrayComplete_Radio = false;			//whether the string is complete

	//data received
	int16_t incomingArray[AIR_DATA_LEN] = { 0 };

	//data to send
	int16_t outgoingArray[AIR_DATA_LEN] = { 0 };




	//initialize the class
	void begin(bool _IFC);

	//send data from outgoingArray
	void sendData();

	//update incomingArray with new data if available
	byte grabData_Radio();

private:// <<---------------------------------------------------------------------------//private

	//serial ports as Stream classes
	Stream *AIR_DEBUG_PORT;
	Stream *AIR_COMMAND_PORT;
	Stream *AIR_TELEM_PORT;

	//variable to determine if this radio is for the IFC or GS
	bool is_IFC;

	//data processing buffers
	byte outBuff[BUFF_LEN] = { 0 };

	//find 8-bit checksum of message
	byte findChecksum(byte buff[]);




	//send the packet one byte at a time
	void writePacket(byte packet[]);

	//unpack and save received data packet
	int extractData_Radio();
};

//create GPS class
extern airComms myRadio;
