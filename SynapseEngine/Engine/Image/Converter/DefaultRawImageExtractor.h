#pragma once
#include "IRawImageExtractor.h"

namespace Syn {
    class SYN_API DefaultRawImageExtractor : public IRawImageExtractor {
    public:
        RawImage Extract(const ImageDownloadResult& downloadData, const Vk::Image& targetImage) override;
    };
}