#pragma once
#include "IImageLoader.h"

namespace Syn
{
    class SYN_API GliImageLoader : public IImageLoader
    {
    public:
        GliImageLoader() = default;
        ~GliImageLoader() override = default;

        std::optional<RawImage> LoadFile(const std::filesystem::path& path) override;
        std::vector<std::string> GetSupportedExtensions() const override;

    private:
        VkFormat GliFormatToVulkan(int gliFormat) const;
    };
}