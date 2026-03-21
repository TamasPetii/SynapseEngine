#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Raw/RawAnimation.h"
#include <optional>
#include <filesystem>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API IAnimationLoader
    {
    public:
        virtual ~IAnimationLoader() = default;
        virtual std::optional<RawAnimation> LoadFile(const std::filesystem::path& path) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}