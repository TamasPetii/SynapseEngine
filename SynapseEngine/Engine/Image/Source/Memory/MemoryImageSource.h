#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Source/IImageSource.h"
#include "Engine/Mesh/Data/Common/MaterialInfo.h"
#include "Engine/Image/Loader/IImageLoader.h"

namespace Syn 
{
    class SYN_API MemoryImageSource : public IImageSource {
    public:
        MemoryImageSource(const TexturePayload& payload, IImageLoader* loader);
        ~MemoryImageSource() override = default;
        std::optional<RawImage> Produce() override;
    private:
        TexturePayload _payload;
        IImageLoader* _loader;
    };
}