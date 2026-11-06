#pragma once

#include "Arduino.h"
#include "Packet.h"
#include "Wire.h"

/**
 * @brief Default maximum bytes written in one Wire transaction.
 *
 * The default fits common 32-byte Wire buffers. Define this macro before
 * including I2CTransfer.h to match the capacity of a different Wire core.
 */
#ifndef SERIALTRANSFER_I2C_CHUNK_SIZE
#define SERIALTRANSFER_I2C_CHUNK_SIZE 32
#endif

#if SERIALTRANSFER_I2C_CHUNK_SIZE < 1 || SERIALTRANSFER_I2C_CHUNK_SIZE > 255
#error "SERIALTRANSFER_I2C_CHUNK_SIZE must be between 1 and 255"
#endif

/**
 * @brief Packetized communication over Arduino Wire.
 *
 * Call Wire.begin() first, then begin(). Large frames are sent across
 * consecutive Wire transactions. The receiver has one payload buffer; poll
 * available() or tick() before the peer sends another packet.
 *
 * Wire does not provide callback context, so this implementation supports one
 * active I2CTransfer instance per MCU at a time.
 */
class I2CTransfer
{
  public:
	/** @brief Packet buffers, parser status, and object serialization helpers. */
	Packet packet;

	/** @brief Payload length from the most recently completed packet. */
	volatile uint8_t bytesRead = 0;

	/** @brief Result of the most recent receive poll; compare with parser status constants. */
	volatile int8_t status = NO_DATA;

	/**
	 * @brief Result of the most recent sendData() call.
	 *
	 * Zero indicates Wire success. Nonzero values are Wire endTransmission()
	 * errors; 0xFF indicates a short Wire.write(). Value 4 is Wire's other-error
	 * result and is also used when no port was initialized. A zero-length packet
	 * also returns zero from sendData(), so inspect this field to distinguish
	 * success from failure.
	 */
	uint8_t transmissionStatus = 0;

	/** @brief Construct an uninitialized transfer object. */
	I2CTransfer() {}

	/**
	 * @brief Initialize with advanced parser, callback, and debug settings.
	 * @param port Wire instance; call port.begin() before this method.
	 * @param configs Parser configuration. The callback array must remain valid
	 *                while this object uses it.
	 */
	void begin(TwoWire& port, const configST& configs);

	/**
	 * @brief Initialize with the default parser timeout and optional diagnostics.
	 * @param port Wire instance; call port.begin() before this method.
	 * @param debug Print parser errors when true.
	 * @param debugPort Stream receiving diagnostics.
	 */
	void begin(TwoWire& port, const bool& debug = true, Stream& debugPort = Serial);

	/**
	 * @brief Build and send one packet to an I2C address.
	 *
	 * Frames are divided into transactions no larger than
	 * SERIALTRANSFER_I2C_CHUNK_SIZE. Address 0 is the I2C general-call address.
	 * @param messageLen Requested payload length; values above 255 are clamped.
	 * @param packetID Application-defined packet identifier.
	 * @param targetAddress Seven-bit I2C target address, or 0 for general call.
	 * @return Number of payload bytes sent; inspect transmissionStatus on error.
	 */
	uint8_t sendData(const uint16_t& messageLen, const uint8_t& packetID = 0,
		const uint8_t& targetAddress = 0);

	/**
	 * @brief Consume a completed packet and dispatch its callback in loop context.
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

	/** @brief Discard a pending packet and reset packet buffers and parser state. */
	void reset();

	/**
	 * @brief Copy an object's raw bytes into the outgoing payload buffer.
	 *
	 * This is a byte-for-byte copy; structure padding and byte order are not
	 * normalized. The copy is clamped to the object and the 255-byte payload.
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
	 * @param packetID Application-defined packet identifier.
	 * @param targetAddress Seven-bit target address, or 0 for general call.
	 * @param len Requested byte count, clamped to the object and payload limits.
	 * @return Number of payload bytes sent; inspect transmissionStatus on error.
	 */
	template <typename T>
	uint8_t sendDatum(const T& value, const uint8_t& packetID = 0,
		const uint8_t& targetAddress = 0, const uint16_t& len = sizeof(T))
	{
		return sendData(packet.txObj(value, 0, len), packetID, targetAddress);
	}

  private:
	static I2CTransfer* classToUse;
	TwoWire* port = NULL;
	const functionPtr* callbacks = NULL;
	uint16_t callbacksLen = 0;
	Stream* debugPort = &Serial;
	bool debug = false;

	volatile bool packetPending = false;
	volatile int8_t deferredError = CONTINUE;

	static void processData(int numBytes);
	void reportError(int8_t error);
};
