#pragma once
#include "IImageLoader.h"
#include <stb_image.h>

namespace Syn
{
    class SYN_API HdriImageLoader : public IImageLoader
    {
    public:
        HdriImageLoader() = default;
        ~HdriImageLoader() override = default;

        std::optional<RawImage> LoadFile(const std::filesystem::path& path) override;
        std::optional<RawImage> LoadMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    private:
        std::optional<RawImage> ProcessData(float* data, int width, int height, int originalChannels);
    };
}