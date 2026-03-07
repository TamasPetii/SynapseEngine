#pragma once
#include "../IImageSource.h"
#include "Engine/Image/Loader/IImageLoader.h"
#include <filesystem>

namespace Syn
{
    class SYN_API FileImageSource : public IImageSource
    {
    public:
        FileImageSource(const std::filesystem::path& path, IImageLoader* loader)
            : _path(path), _loader(loader) {}

        std::optional<RawImage> Produce() override;
    private:
        std::filesystem::path _path;
        IImageLoader* _loader;
    };
}