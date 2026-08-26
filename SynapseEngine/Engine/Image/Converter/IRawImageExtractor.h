#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Raw/RawImage.h"
#include "Engine/Image/Downloader/IGpuImageDownloader.h"
#include "Engine/Vk/Image/Image.h"

namespace Syn 
{
    class SYN_API IRawImageExtractor {
    public:
        virtual ~IRawImageExtractor() = default;
        virtual RawImage Extract(const ImageDownloadResult& downloadData, const Vk::Image& targetImage) = 0;
    };
}