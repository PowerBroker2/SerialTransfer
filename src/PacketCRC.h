#pragma once

#include <stddef.h>
#include <stdint.h>


template <uint8_t crcLen = 8, uint8_t polynomial = 0x9B>
class PacketCRC
{
  public: // <<---------------------------------------//public
	static inline constexpr const uint8_t calculate(const uint8_t& val)
	{
		return (val < tableLen) ? crcTable[val] : 0;
	}


	// This function can be constexpr in C++14 and above!
	static inline
#if __cplusplus > 201103L
	    constexpr
#endif
	    const uint8_t
	    calculate(const uint8_t arr[], uint8_t len)
	{
		uint8_t crc = 0;

		for (uint8_t i = 0; i < len; i++)
			crc = calculate(crc ^ arr[i]);

		return crc;
	}

	PacketCRC()                 = delete;
	PacketCRC(const PacketCRC&) = delete;
	PacketCRC(PacketCRC&&)      = delete;


  private: // <<---------------------------------------//private
	       // ============================================
	       // Taken and adjusted from: https://github.com/BrainStone/CppCompiletimeArrayGenerator
	template <size_t N, uint8_t (*FUNC)(size_t)>
	struct Generator
	{
		static_assert(N > 0, "N must be greater than 0");

	  private:
		template <size_t M, uint8_t... Rest>
		struct Generator_impl
		{
			static constexpr const uint8_t (&value)[N] = Generator_impl<M - 1, FUNC(M), Rest...>::value;
		};

		template <uint8_t... Rest>
		struct Generator_impl<0, Rest...>
		{
			static constexpr const uint8_t value[N] = {FUNC(0), Rest...};
		};

	  public:
		static constexpr const uint8_t (&value)[N] = Generator_impl<N - 1>::value;

		Generator()                 = delete;
		Generator(const Generator&) = delete;
		Generator(Generator&&)      = delete;
	};


	// ============================================


	static inline constexpr uint8_t step(uint8_t curr)
	{
		return (curr << 1) ^ (((curr & 0x80) != 0) ? polynomial : 0);
	}


	static inline constexpr uint8_t step(uint8_t curr, size_t depth)
	{
		return (depth > 1) ? step(step(curr), depth - 1) : step(curr);
	}


	static inline constexpr uint8_t generateValue(size_t index)
	{
		// No loops for C++11 constexpr functions
		// Essentially we compute
		// curr = (curr << 1) ^ (((curr & 0x80) != 0) ? polynomial : 0)
		// 8 times
		return step(static_cast<uint8_t>(index), 8);
	}


	static constexpr const uint16_t tableLen             = 1 << crcLen;
	static constexpr const uint8_t (&crcTable)[tableLen] = Generator<tableLen, PacketCRC<crcLen, polynomial>::generateValue>::value;
};

template <uint8_t crcLen, uint8_t polynomial>
constexpr const uint8_t (&PacketCRC<crcLen, polynomial>::crcTable)[PacketCRC<crcLen, polynomial>::tableLen];

template <uint8_t crcLen, uint8_t polynomial>
template <size_t N, uint8_t (*FUNC)(size_t)>
template <uint8_t... Rest>
constexpr const uint8_t PacketCRC<crcLen, polynomial>::Generator<N, FUNC>::Generator_impl<0, Rest...>::value[];
