#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Raw/RawImage.h"
#include <filesystem>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API IImageWriter {
    public:
        virtual ~IImageWriter() = default;
        virtual bool WriteFile(const std::filesystem::path& path, const RawImage& image) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}