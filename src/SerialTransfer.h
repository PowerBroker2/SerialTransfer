#pragma once

#include "Arduino.h"
#include "Packet.h"

/**
 * @brief Packetized communication over any Arduino Stream.
 *
 * The frame is `START | ID | LENGTH | PAYLOAD | CRC-8`. Call available() or
 * tick() regularly to parse received bytes. The 255-byte payload and packet
 * buffers are owned by the contained Packet object.
 */
class SerialTransfer
{
  public:
	/** @brief Packet buffers, parser status, and object serialization helpers. */
	Packet packet;

	/** @brief Payload length from the most recently completed packet. */
	uint8_t bytesRead = 0;

	/** @brief Result of the most recent receive poll; compare with parser status constants. */
	int8_t status = NO_DATA;

	/**
	 * @brief Initialize with advanced parser, callback, and debug settings.
	 * @param port Stream used for both transmit and receive.
	 * @param configs Parser configuration. The callback array must remain valid
	 *                while this object uses it.
	 */
	void begin(Stream& port, const configST configs);

	/**
	 * @brief Initialize with a timeout and optional parser diagnostics.
	 * @param port Stream used for both transmit and receive.
	 * @param debug Print parser errors when true.
	 * @param debugPort Stream receiving diagnostics.
	 * @param timeoutMs Inter-byte timeout in milliseconds; UINT32_MAX disables it.
	 */
	void begin(Stream& port, const bool debug = true, Stream& debugPort = Serial,
		uint32_t timeoutMs = DEFAULT_TIMEOUT);

	/**
	 * @brief Build and write one packet.
	 * @param messageLen Requested payload length; values above 255 are clamped.
	 * @param packetID Application-defined packet identifier.
	 * @return Number of payload bytes placed in the transmitted packet.
	 */
	uint8_t sendData(const uint16_t& messageLen, const uint8_t packetID = 0);

	/**
	 * @brief Parse available input bytes and dispatch any matching callback.
	 * @return Payload length for a completed packet, otherwise zero. Check status
	 *         to distinguish no packet from a valid zero-length packet or error.
	 */
	uint8_t available();

	/**
	 * @brief Poll for input and report whether a packet completed.
	 * @return True only when this call processed a complete packet.
	 */
	bool tick();

	/** @brief Return the ID of the most recently completed packet. */
	uint8_t currentPacketID();

	/** @brief Discard unread stream input and reset packet buffers and parser state. */
	void reset();

	/**
	 * @brief Copy an object's raw bytes into the outgoing payload buffer.
	 *
	 * This is a byte-for-byte copy: structure padding, endianness, and floating
	 * point representation are not normalized. Use matching layouts at both ends
	 * or serialize fields explicitly. The copy is clamped to the object and the
	 * 255-byte payload boundary.
	 * @param value Object whose bytes are copied.
	 * @param index Destination payload offset.
	 * @param len Requested byte count.
	 * @return Payload offset immediately after the copied bytes.
	 */
	template <typename T>
	uint16_t txObj(const T& value, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		return packet.txObj(value, index, len);
	}

	/**
	 * @brief Copy received payload bytes into an object.
	 *
	 * This is a byte-for-byte copy. A short payload copies only available bytes
	 * and leaves the remainder of value unchanged; check bytesRead before use.
	 * @param value Destination object.
	 * @param index Source payload offset.
	 * @param len Requested byte count.
	 * @return Payload offset immediately after the copied bytes.
	 */
	template <typename T>
	uint16_t rxObj(T& value, const uint16_t& index = 0, const uint16_t& len = sizeof(T))
	{
		return packet.rxObj(value, index, len);
	}

	/**
	 * @brief Copy an object to the payload buffer and immediately send it.
	 * @param value Object whose raw bytes are sent.
	 * @param len Requested byte count, clamped to the object and payload limits.
	 * @return Number of payload bytes sent.
	 */
	template <typename T>
	uint8_t sendDatum(const T& value, const uint16_t& len = sizeof(T))
	{
		return sendData(packet.txObj(value, 0, len));
	}

  private:
	Stream* port = NULL;
};
