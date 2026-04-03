#include "DefaultImageCooker.h"

namespace Syn
{
    CookedImage DefaultImageCooker::Cook(const RawImage& rawImage)
    {
        CookedImage cookedImage{};

        cookedImage.width = rawImage.width;
        cookedImage.height = rawImage.height;
        cookedImage.depth = rawImage.depth;
        cookedImage.format = rawImage.format;
        cookedImage.mipLevels = rawImage.mipLevels;
        cookedImage.isCompressed = rawImage.isCompressed;
        cookedImage.pixels = rawImage.pixels;
        cookedImage.mipData = rawImage.mipData;

        if (!cookedImage.isCompressed && cookedImage.mipLevels == 1) {
            cookedImage.autoGenerateMipmaps = true;
        }
        else {
            cookedImage.autoGenerateMipmaps = false;
        }

        return cookedImage;
    }
}