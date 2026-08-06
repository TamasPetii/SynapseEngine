#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include <memory>

namespace Syn 
{
    struct SYN_API StaleImage {
        std::unique_ptr<Vk::Image> image;
        uint32_t framesToLive;
    };
}