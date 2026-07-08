#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    struct SYN_API HierarchyLevelData
    {
        uint32_t startIndex = 0;
        uint32_t capacity = 0;
        uint32_t activeCount = 0;
    };
}