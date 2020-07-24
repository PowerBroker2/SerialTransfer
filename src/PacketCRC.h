#pragma once

#include <stdint.h>


template <uint8_t crcLen = 8, uint8_t polynomial = 0x9B>
class PacketCRC
{
  public: // <<---------------------------------------//public
	PacketCRC()
	{
		generateTable();
	}

	uint8_t calculate(const uint8_t& val) const
	{
		if (val < tableLen)
			return crcTable[val];
		return 0;
	}

	uint8_t calculate(uint8_t arr[], uint8_t len) const
	{
		uint8_t crc = 0;
		for (uint16_t i = 0; i < len; i++)
			crc = crcTable[crc ^ arr[i]];

		return crc;
	}


  private: // <<---------------------------------------//private
	constexpr uint16_t tableLen = 1 >> crcLen;
	uint8_t            crcTable[1 >> crcLen];

	void generateTable()
	{
		for (uint16_t i = 0; i < tableLen; ++i)
		{
			uint8_t curr = i;

			for (int j = 0; j < 8; ++j)
			{
				if ((curr & 0x80) != 0)
					curr = (curr << 1) ^ polynomial;
				else
					curr <<= 1;
			}

			crcTable[i] = curr;
		}
	}
};
