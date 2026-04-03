#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"

namespace Syn
{
    struct SYN_API Texture {
        CookedImage cpuData;
        GpuImage gpuData;
        std::shared_ptr<Vk::Image> image;
    };
}