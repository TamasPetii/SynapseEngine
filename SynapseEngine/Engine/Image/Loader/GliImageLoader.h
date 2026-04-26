#pragma once
#include "IImageLoader.h"
#include <gli/gli.hpp>

namespace Syn
{
    class SYN_API GliImageLoader : public IImageLoader
    {
    public:
        GliImageLoader() = default;
        ~GliImageLoader() override = default;

        std::optional<RawImage> LoadFile(const std::filesystem::path& path) override;
        std::optional<RawImage> LoadMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    private:
        std::optional<RawImage> ProcessData(const gli::texture& texture, const std::string& debugName);
        VkFormat GliFormatToVulkan(int gliFormat) const;
    };
}