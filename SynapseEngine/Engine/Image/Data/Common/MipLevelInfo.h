#pragma once
#include "Engine/SynApi.h"
#include <vector>


namespace Syn
{
    struct SYN_API MipLevelInfo {
        uint32_t width;
        uint32_t height;
        uint32_t size;
        uint32_t offset;
    };
}