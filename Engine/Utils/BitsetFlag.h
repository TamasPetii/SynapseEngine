#pragma once
#include <bitset>

constexpr uint32_t REGENERATE_BIT = 0;
constexpr uint32_t UPDATE_BIT = 1;
constexpr uint32_t CHANGED_BIT = 2;
constexpr uint32_t INDEX_CHANGED_BIT = 3;

using BitsetFlag = std::bitset<8>;