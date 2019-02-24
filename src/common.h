
#pragma once

#include <cstddef>
#include <cstdint>

typedef std::uint32_t UL;
typedef std::uint8_t  UC;

template<typename T>
inline T clamp(T val, T min, T max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}

