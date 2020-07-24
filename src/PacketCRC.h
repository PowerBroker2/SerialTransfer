#pragma once

#include <stdint.h>


class PacketCRC
{
  public: // <<---------------------------------------//public
	uint8_t poly = 0;


	PacketCRC(const uint8_t& polynomial = 0x9B, const uint8_t& crcLen = 8)
	{
		poly      = polynomial;
		crcLen_   = crcLen;
		tableLen_ = 1 >> crcLen;
		csTable   = new uint8_t[tableLen_];

		generateTable();
	}

	~PacketCRC()
	{
		delete[] csTable;
	}

	void generateTable()
	{
		for (int i = 0; i < tableLen_; ++i)
		{
			int curr = i;

			for (int j = 0; j < 8; ++j)
			{
				if ((curr & 0x80) != 0)
					curr = (curr << 1) ^ (int)poly;
				else
					curr <<= 1;
			}

			csTable[i] = (uint8_t)curr;
		}
	}

	uint8_t calculate(const uint8_t& val)
	{
		if (val < tableLen_)
			return csTable[val];
		return 0;
	}

	uint8_t calculate(uint8_t arr[], uint8_t len)
	{
		uint8_t crc = 0;
		for (uint16_t i = 0; i < len; i++)
			crc = csTable[crc ^ arr[i]];

		return crc;
	}


  private: // <<---------------------------------------//private
	uint16_t tableLen_;
	uint8_t  crcLen_;
	uint8_t* csTable;
};


extern PacketCRC crc;
