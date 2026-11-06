#pragma once
#include "Arduino.h"


/** @brief Lookup-table implementation of the library's CRC-8 calculation. */
class PacketCRC
{
  public: // <<---------------------------------------//public
	/** @brief Table polynomial (default 0x9B); call generateTable() after changing it. */
	uint8_t poly = 0;


	/**
	 * @brief Construct a CRC lookup table.
	 * @param polynomial CRC polynomial; defaults to 0x9B.
	 * @param crcLen Retained for source compatibility; this implementation is CRC-8 and always uses 8 bits.
	 */
	PacketCRC(const uint8_t& polynomial = 0x9B, const uint8_t& crcLen = 8)
	{
		poly      = polynomial;
		(void)crcLen;
		tableLen_ = 256;

		generateTable();
	}

	/** @brief Generate the table for the configured polynomial. */
	void generateTable()
	{
		for (uint16_t i = 0; i < tableLen_; ++i)
		{
			int curr = i;

			for (int j = 0; j < 8; ++j)
			{
				if ((curr & 0x80) != 0)
					curr = (curr << 1) ^ (int)poly;
				else
					curr <<= 1;
			}

			csTable[i] = (byte)curr;
		}
	}

	/** @brief Print the lookup table to the global Serial stream for diagnostics. */
	void printTable()
	{
		for (uint16_t i = 0; i < tableLen_; i++)
		{
			Serial.print(csTable[i], HEX);

			if ((i + 1) % 16)
				Serial.print(' ');
			else
				Serial.println();
		}
	}

	/** @brief Return the table entry for one byte value, or zero if out of range. */
	uint8_t calculate(const uint8_t& val)
	{
		if (val < tableLen_)
			return csTable[val];
		return 0;
	}

	/**
	 * @brief Update an existing CRC with one byte.
	 * @param current Current CRC accumulator.
	 * @param val Next input byte.
	 * @return Updated CRC accumulator.
	 */
	inline uint8_t update(const uint8_t current, const uint8_t val)
	{
		return csTable[current ^ val];
	}

	/**
	 * @brief Calculate CRC-8 over a byte array, starting with accumulator zero.
	 * @param arr Input bytes.
	 * @param len Number of bytes to process.
	 * @return Calculated CRC.
	 */
	uint8_t calculate(const uint8_t* arr, const uint16_t len)
	{
		uint8_t crc = 0;
		for (uint16_t i = 0; i < len; i++)
			crc = update(crc, arr[i]);

		return crc;
	}


  private: // <<---------------------------------------//private
	uint16_t tableLen_;
	uint8_t  csTable[256];
};


/** @brief Shared CRC-8 lookup table used by packet construction and parsing. */
extern PacketCRC crc;
