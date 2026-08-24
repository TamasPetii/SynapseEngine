#pragma once
#include "Engine/SynApi.h"
#include "IImageWriter.h"
#include <vulkan/vulkan.h>

namespace Syn
{
    class SYN_API StbImageWriter : public IImageWriter
    {
    public:
        StbImageWriter() = default;
        ~StbImageWriter() override = default;

        bool WriteFile(const std::filesystem::path& path, const RawImage& image) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}