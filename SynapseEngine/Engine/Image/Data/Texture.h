#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Image/Data/Cooked/CookedImage.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"
#include "Engine/Image/Data/Cpu/CpuTextureData.h"

namespace Syn
{
    struct SYN_API Texture {
        CpuTextureData cpuData;
        std::shared_ptr<Vk::Image> image;

        std::unique_ptr<CookedImage> transientCpuData;
        std::unique_ptr<GpuImage> transientGpuData;
    };
}