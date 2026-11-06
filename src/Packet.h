/** @file Packet framing format: START | ID | LENGTH | PAYLOAD | CRC-8(ID, LENGTH, PAYLOAD). */

#pragma once
#include "Arduino.h"
#include "PacketCRC.h"
#include <string.h>


/** @brief Callback signature used by configST; callbacks take no arguments. */
typedef void (*functionPtr)();


/** @brief Parser is waiting for more bytes in the current frame. */
const int8_t CONTINUE           = 3;
/** @brief A valid packet was parsed. */
const int8_t NEW_DATA           = 2;
/** @brief No complete packet was available. */
const int8_t NO_DATA            = 1;
/** @brief Received CRC did not match the calculated CRC. */
const int8_t CRC_ERROR          = 0;
/** @brief Reserved parser error code. */
const int8_t PAYLOAD_ERROR      = -1;
/** @brief Reserved for compatibility; current framing has no stop byte. */
const int8_t STOP_BYTE_ERROR    = -2;
/** @brief An incomplete packet exceeded its configured inter-byte timeout. */
const int8_t STALE_PACKET_ERROR = -3;

/** @brief Frame synchronization byte. */
const uint8_t START_BYTE = 0x7E;
/** @brief Number of header bytes: start marker, ID, and payload length. */
const uint8_t PREAMBLE_SIZE   = 3;
/** @brief Number of CRC bytes at the end of each frame. */
const uint8_t POSTAMBLE_SIZE  = 1;
/** @brief Maximum payload length, in bytes. */
const uint8_t MAX_PACKET_SIZE = 0xFF;

/** @brief Default inter-byte parser timeout, in milliseconds. */
const uint8_t DEFAULT_TIMEOUT = 50;


/**
 * @brief Advanced packet parser configuration.
 *
 * The callbacks array is indexed directly by packet ID. Null entries are
 * skipped; callbacksLen can describe all 256 packet IDs. Keep the array alive
 * for the lifetime of the configured transfer object.
 */
struct configST
{
	/** @brief Stream for optional parser diagnostics. */
	Stream*            debugPort    = &Serial;
	/** @brief Enable parser diagnostic messages. */
	bool               debug        = true;
	/** @brief Callback table indexed by packet ID, or NULL to disable callbacks. */
	const functionPtr* callbacks    = NULL;
	/** @brief Number of valid entries in callbacks (0 through 256 packet IDs). */
	uint16_t           callbacksLen = 0;
	/** @brief Inter-byte timeout in milliseconds; UINT32_MAX disables timeout checks. */
	uint32_t           timeout      = __UINT32_MAX__;
};


/**
 * @brief Owns packet buffers and the shared frame parser.
 *
 * Transfer classes expose this object as `packet`; most applications should
 * use the transport's txObj(), rxObj(), sendData(), and available() methods.
 */
class Packet
{
  public: // <<---------------------------------------//public
	/** @brief Outgoing payload buffer plus one byte for its CRC. */
	uint8_t txBuff[MAX_PACKET_SIZE + POSTAMBLE_SIZE];
	/** @brief Incoming payload buffer, valid after a NEW_DATA result. */
	uint8_t rxBuff[MAX_PACKET_SIZE];
	/** @brief Current outgoing header: start marker, packet ID, and payload length. */
	uint8_t preamble[PREAMBLE_SIZE]   = {START_BYTE, 0, 0};
	/** @brief Current outgoing CRC byte. */
	uint8_t postamble[POSTAMBLE_SIZE] = {0};

	/** @brief Length of the most recently completed incoming payload. */
	uint8_t bytesRead = 0;
	/** @brief Result from the most recent parser operation. */
	int8_t  status    = NO_DATA;


	/** @brief Apply advanced callbacks, diagnostics, and timeout configuration. */
	void    begin(const configST& configs);
	/** @brief Configure optional diagnostics and the inter-byte timeout. */
	void    begin(const bool& _debug = true, Stream& _debugPort = Serial, const uint32_t& _timeout = DEFAULT_TIMEOUT);
	/**
	 * @brief Build the outgoing header and calculate its CRC-8.
	 * @param messageLen Requested payload length; values above 255 are clamped.
	 * @param packetID Application-defined identifier.
	 * @return Payload length included in the frame.
	 */
	uint8_t constructPacket(const uint16_t& messageLen, const uint8_t& packetID = 0);
	/**
	 * @brief Feed one received byte to the parser.
	 * @param recChar Byte from the transport.
	 * @param valid False when polling found no byte; permits timeout handling.
	 * @return Payload length when a valid packet completes, otherwise zero.
	 */
	uint8_t parse(const uint8_t& recChar, const bool& valid = true);
	/** @brief Return the ID of the most recently completed packet. */
	uint8_t currentPacketID();
	/** @brief Clear payload buffers and reset the parser to its idle state. */
	void    reset();


	/**
	 * @brief Copy an object's raw bytes into the outgoing payload buffer.
	 *
	 * Structure padding, endianness, and floating-point representation are not
	 * normalized. The copy is clamped to sizeof(T) and the payload boundary.
	 * @param val Source object.
	 * @param index Destination payload offset.
	 * @param len Requested number of bytes.
	 * @return Payload offset immediately after the copied bytes.
	 */
	template <typename T>
	uint16_t txObj(const T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		if (index >= MAX_PACKET_SIZE)
			return MAX_PACKET_SIZE;

		uint32_t endIndex = (uint32_t)index + len;
		if (endIndex > MAX_PACKET_SIZE)
			endIndex = MAX_PACKET_SIZE;

		uint16_t copyLen = (uint16_t)(endIndex - index);
		if (copyLen > sizeof(T))
			copyLen = sizeof(T);

		if (copyLen)
			memcpy(txBuff + index, &val, copyLen);

		return index + copyLen;
	}


	/**
	 * @brief Copy incoming payload bytes into an object.
	 *
	 * Structure padding and byte order are not normalized. A short copy leaves
	 * the remaining destination bytes unchanged; check bytesRead first.
	 * @param val Destination object.
	 * @param index Source payload offset.
	 * @param len Requested number of bytes.
	 * @return Payload offset immediately after the copied bytes.
	 */
	template <typename T>
	uint16_t rxObj(T& val, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		if (index >= bytesRead || index >= MAX_PACKET_SIZE)
			return index >= MAX_PACKET_SIZE ? MAX_PACKET_SIZE : index;

		uint32_t endIndex = (uint32_t)index + len;
		if (endIndex > bytesRead)
			endIndex = bytesRead;

		uint16_t copyLen = (uint16_t)(endIndex - index);
		if (copyLen > sizeof(T))
			copyLen = sizeof(T);

		if (copyLen)
			memcpy(&val, rxBuff + index, copyLen);

		return index + copyLen;
	}


  private: // <<---------------------------------------//private
	enum fsm
	{
		find_start_byte,
		find_id_byte,
		find_payload_len,
		find_payload,
		find_crc
	};
	fsm state = find_start_byte;

	const functionPtr* callbacks    = NULL;
	uint16_t           callbacksLen = 0;

	Stream* debugPort;
	bool    debug = false;

	uint8_t  bytesToRec = 0;
	uint16_t payIndex   = 0;
	uint8_t  idByte     = 0;
	uint8_t  calcCrc    = 0;

	uint32_t lastByteTime = 0;
	uint32_t timeout      = DEFAULT_TIMEOUT;


	void resetParserState();
};
