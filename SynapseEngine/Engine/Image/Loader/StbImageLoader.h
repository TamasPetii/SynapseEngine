#pragma once
#include "IImageLoader.h"

namespace Syn
{
    class SYN_API StbImageLoader : public IImageLoader
    {
    public:
        StbImageLoader() = default;
        ~StbImageLoader() override = default;

        std::optional<RawImage> LoadFile(const std::filesystem::path& path) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}