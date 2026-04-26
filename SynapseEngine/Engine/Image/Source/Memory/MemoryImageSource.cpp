#include "MemoryImageSource.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    MemoryImageSource::MemoryImageSource(const TexturePayload& payload, IImageLoader* loader)
        : _payload(payload), _loader(loader) {

    }

    std::optional<RawImage> MemoryImageSource::Produce() {
        if (_payload.embeddedData.empty()) 
            return std::nullopt;

        if (!_payload.isUncompressed) {
            if (!_loader) {
                Error("MemoryImageSource: No loader provided for compressed embedded image!");
                return std::nullopt;
            }
            return _loader->LoadMemory(_payload.embeddedData);
        }
        else {
            RawImage image{};
            image.width = _payload.width;
            image.height = _payload.height;
            image.depth = 1;
            image.mipLevels = 1;
            image.isCompressed = false;
            image.format = VK_FORMAT_R8G8B8A8_UNORM;

            image.pixels = _payload.embeddedData;
            
            for (size_t i = 0; i < image.pixels.size(); i += 4) {
                uint8_t b = image.pixels[i];
                uint8_t g = image.pixels[i + 1];
                uint8_t r = image.pixels[i + 2];
                uint8_t a = image.pixels[i + 3];
                image.pixels[i] = r;
                image.pixels[i + 1] = g;
                image.pixels[i + 2] = b;
                image.pixels[i + 3] = a;
            }

            MipLevelInfo mip0{};
            mip0.width = image.width;
            mip0.height = image.height;
            mip0.size = static_cast<uint32_t>(image.pixels.size());
            mip0.offset = 0;
            image.mipData.push_back(mip0);

            return image;
        }
    }
}