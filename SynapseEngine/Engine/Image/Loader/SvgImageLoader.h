#pragma once
#include "IImageLoader.h"

struct NSVGimage;

namespace Syn
{
    class SYN_API SvgImageLoader : public IImageLoader
    {
    public:
        SvgImageLoader() = default;
        ~SvgImageLoader() override = default;

        std::optional<RawImage> LoadFile(const std::filesystem::path& path) override;
        std::optional<RawImage> LoadMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;

    private:
        std::optional<RawImage> ProcessImage(struct NSVGimage* image);
    };
}