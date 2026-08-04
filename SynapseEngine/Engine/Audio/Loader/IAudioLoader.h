#pragma once
#include "Engine/SynApi.h"
#include "Engine/Audio/Data/Raw/RawAudio.h"
#include <optional>
#include <filesystem>
#include <string>
#include <vector>

namespace Syn
{
    class SYN_API IAudioLoader {
    public:
        virtual ~IAudioLoader() = default;
        virtual std::optional<RawAudio> LoadFile(const std::filesystem::path& path) = 0;
        virtual std::optional<RawAudio> LoadMemory(const std::vector<uint8_t>& data) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}