#include "DefaultImageSource.h"

namespace Syn
{
    DefaultImageSource::DefaultImageSource()
        : ProceduralImageSource("DefaultFallbackTexture")
    {}

    std::optional<RawImage> DefaultImageSource::Produce()
    {
        RawImage image{};

        image.width = 1;
        image.height = 1;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R8G8B8A8_UNORM;
        image.isCompressed = false;
        image.pixels = { 255, 255, 255, 255 };

        MipLevelInfo mip0{};
        mip0.width = 1;
        mip0.height = 1;
        mip0.size = 4;
        mip0.offset = 0;

        image.mipData.push_back(mip0);
        return image;
    }
}