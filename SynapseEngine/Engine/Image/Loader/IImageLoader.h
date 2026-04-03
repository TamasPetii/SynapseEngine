#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Data/Raw/RawImage.h"
#include <optional>
#include <filesystem>
#include <string>
#include <vector>

namespace Syn
{
    class SYN_API IImageLoader {
    public:
        virtual ~IImageLoader() = default;
        virtual std::optional<RawImage> LoadFile(const std::filesystem::path& path) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}