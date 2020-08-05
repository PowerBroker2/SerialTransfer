/*
01111110 00000000 11111111 00000000 00000000 00000000 ... 00000000 10000001
|      | |      | |      | |      | |      | |      | | | |      | |______|__Stop byte
|      | |      | |      | |      | |      | |      | | | |______|___________8-bit CRC
|      | |      | |      | |      | |      | |      | |_|____________________Rest of payload
|      | |      | |      | |      | |      | |______|________________________2nd payload byte
|      | |      | |      | |      | |______|_________________________________1st payload byte
|      | |      | |      | |______|__________________________________________# of payload bytes
|      | |      | |______|___________________________________________________COBS Overhead byte
|      | |______|____________________________________________________________Packet ID (0 by default)
|______|_____________________________________________________________________Start byte (constant)
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "PacketCRC.h"


enum ParserState : int8_t
{
	CONTINUE        = 2,
	NEW_DATA        = 1,
	NO_DATA         = 0,
	CRC_ERROR       = -1,
	PAYLOAD_ERROR   = -2,
	STOP_BYTE_ERROR = -3
};

constexpr uint8_t START_BYTE = 0x7E;
constexpr uint8_t STOP_BYTE  = 0x81;

constexpr uint8_t PREAMBLE_SIZE   = 4;
constexpr uint8_t POSTAMBLE_SIZE  = 2;
constexpr uint8_t MAX_PACKET_SIZE = 0xFE; // Maximum allowed payload bytes per packet
constexpr uint8_t AUTO_INDEX      = 0xFF;


class Packet
{
  public: // <<---------------------------------------//public
	typedef void (*CallbackFunc)(Packet&);


	uint8_t (&txBuff)[MAX_PACKET_SIZE] = _txBuff;
	uint8_t rxBuff[MAX_PACKET_SIZE];

	uint8_t bytesToSend = 0;
	uint8_t bytesRead   = 0;


	uint8_t sendPacket(uint8_t packetID = 0);


	/*
	 uint16_t Packet::txObj(const T& val, size_t len = sizeof(T), uint8_t index = AUTO_INDEX)
	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index"

	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * const uint16_t &index - Starting index of the object within the
	  transmit buffer (txBuff)
	  * const uint16_t &len - Number of bytes of the object "val" to transmit

	 Return:
	 -------
	  * uint16_t maxIndex - uint16_t maxIndex - Index of the transmit buffer (txBuff) that directly follows the bytes processed
	  by the calling of this member function
	*/
	template <typename T>
	uint8_t txObj(const T& val, size_t len = sizeof(T), uint8_t index = AUTO_INDEX)
	{
		const uint8_t* ptr        = reinterpret_cast<const uint8_t*>(&val);
		const uint8_t  indexLimit = MAX_PACKET_SIZE;
		uint8_t        maxIndex;

		if (index == AUTO_INDEX)
			index = bytesToSend;

		// Doing it like this makes sure that the comparison is done with the data type size_t instead of uint8_t, which could overflow!
		maxIndex = ((len + index) > indexLimit) ? indexLimit : (len + index);

		for (uint8_t i = index; i < maxIndex; i++)
			txBuff[i] = *(ptr++);

		if (bytesToSend < maxIndex)
			bytesToSend = maxIndex;

		return maxIndex;
	}


	/*
	 uint8_t Packet::sendObj(const T& val, uint8_t packetID = 0, size_t len = sizeof(T))
	 Description:
	 ------------
	  * Stuffs "len" number of bytes of an arbitrary object (byte, int,
	  float, double, struct, etc...) into the transmit buffer (txBuff)
	  starting at the index as specified by the argument "index" and
	  automatically transmits the bytes in an individual packet
	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied to the
	  transmit buffer (txBuff)
	  * const uint16_t &len - Number of bytes of the object "val" to transmit
	 Return:
	 -------
	  * uint8_t - Number of payload bytes included in packet
	*/
	template <typename T>
	uint8_t sendObj(const T& val, uint8_t packetID = 0, size_t len = sizeof(T))
	{
		if (debug && (bytesToSend != 0))
			printDebug("Discarded existing data during call to Packet::sendObj!");

		// Discard any other data
		bytesToSend = 0;
		txObj(val, len);

		return sendPacket(packetID);
	}

	uint8_t available();
	bool    tick();
	uint8_t getPacketID() const;
	uint8_t getPacketSize() const;


	/*
	 uint16_t Packet::rxObj(T& val, size_t len = sizeof(T), uint8_t index = AUTO_INDEX)
	 Description:
	 ------------
	  * Reads "len" number of bytes from the receive buffer (rxBuff)
	  starting at the index as specified by the argument "index"
	  into an arbitrary object (byte, int, float, double, struct, etc...)

	 Inputs:
	 -------
	  * const T &val - Pointer to the object to be copied into from the
	  receive buffer (rxBuff)
	  * const uint16_t &index - Starting index of the object within the
	  receive buffer (rxBuff)
	  * const uint16_t &len - Number of bytes in the object "val" received

	 Return:
	 -------
	  * uint16_t maxIndex - Index of the receive buffer (rxBuff) that directly follows the bytes processed
	  by the calling of this member function
	*/
	template <typename T>
	uint8_t rxObj(T& val, size_t len = sizeof(T), uint8_t index = AUTO_INDEX)
	{
		uint8_t*      ptr        = reinterpret_cast<uint8_t*>(&val);
		const uint8_t indexLimit = bytesRec;
		uint8_t       maxIndex;

		if (index == AUTO_INDEX)
			index = bytesRead;

		// Doing it like this makes sure that the comparison is done with the data type size_t instead of uint8_t, which could overflow!
		maxIndex = ((len + index) > indexLimit) ? indexLimit : (len + index);

		for (uint8_t i = index; i < maxIndex; i++)
			*(ptr++) = rxBuff[i];

		if (bytesRead < maxIndex)
			bytesRead = maxIndex;

		return maxIndex;
	}

	ParserState getStatus() const;

	void addCallback(CallbackFunc callback);


  protected: // <<---------------------------------------//protected
	Packet(bool debug = false);
	virtual ~Packet();

	void begin(bool debug = false);

	union
	{
		uint8_t txRawBuff[PREAMBLE_SIZE + MAX_PACKET_SIZE + POSTAMBLE_SIZE];
		struct
		{
			uint8_t preamble[PREAMBLE_SIZE] = {START_BYTE, 0, 0, 0};
			uint8_t _txBuff[MAX_PACKET_SIZE];
			// Postamble missing on purpose because it doesn't have a static position
		} __attribute__((packed));
	};

	enum class fsm
	{
		find_start_byte,
		find_id_byte,
		find_overhead_byte,
		find_payload_len,
		find_payload,
		find_crc,
		find_end_byte
	};

	ParserState status = NO_DATA;
	fsm         state  = fsm::find_start_byte;

	// Vritual functions to override
	virtual bool    bytesAvailable() = 0;
	virtual uint8_t readByte()       = 0;
	virtual void    writeBytes()     = 0;
	virtual void    printDebug(const char* msg) const;


  private: // <<---------------------------------------//private
	struct CallbackNode
	{
		CallbackNode*      next;
		const CallbackFunc callback;

		inline CallbackNode(CallbackFunc callback) : next(nullptr), callback(callback){};
	};

	bool debug;

	CallbackNode* callbacks = nullptr;

	// Internal parser state
	uint8_t bytesRec        = 0;
	uint8_t bytesToRec      = 0;
	uint8_t payIndex        = 0;
	uint8_t idByte          = 0;
	uint8_t recOverheadByte = 0;


	uint8_t stuffPacket();
	void    unpackPacket();
	void    callCallbacks();
};
