#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>

namespace Syn
{
    struct SYN_API VideoUploadResult {
        std::shared_ptr<Vk::Image> texture;
        std::unique_ptr<Vk::Buffer> bitstreamBuffer;
        bool isFrameReady = false;
    };
}