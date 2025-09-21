#pragma once
#include "BitsetFlag.h"
#include <mutex>

class BitsetFlagged
{
public:
	BitsetFlagged();

	template<uint32_t... bitIndex>
	bool IsBitSet();
	template<uint32_t... bitIndex>
	void SetBit();
	template<uint32_t... bitIndex>
	void ResetBit();
private:
	BitsetFlag bitset;
	std::mutex asyncMutex;
};

template<uint32_t ...bitIndex>
inline bool BitsetFlagged::IsBitSet()
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	return (bitset.test(bitIndex) && ...);
}

template<uint32_t ...bitIndex>
inline void BitsetFlagged::SetBit()
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	(bitset.set(bitIndex, true), ...);
}

template<uint32_t ...bitIndex>
inline void BitsetFlagged::ResetBit()
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	(bitset.set(bitIndex, false), ...);
}
