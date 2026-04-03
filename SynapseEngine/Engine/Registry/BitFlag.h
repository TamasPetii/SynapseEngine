#pragma once
#include <cstdint>

namespace Syn {
	constexpr uint32_t REGENERATE_BIT = 0;
	constexpr uint32_t UPDATE_BIT = 1;
	constexpr uint32_t CHANGED_BIT = 2;
	constexpr uint32_t INDEX_CHANGED_BIT = 3;
	constexpr uint32_t DIRTY_STATIC_BIT = 4;
	constexpr uint32_t CUSTOM_CHANGED_BIT1 = 5;
	constexpr uint32_t CUSTOM_CHANGED_BIT2 = 6;
	constexpr uint32_t CUSTOM_CHANGED_BIT3 = 7;
}
