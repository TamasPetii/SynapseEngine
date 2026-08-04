#pragma once
#include "IAudioLoader.h"

namespace Syn
{
    class SYN_API MiniAudioLoader : public IAudioLoader
    {
    public:
        MiniAudioLoader() = default;
        ~MiniAudioLoader() override = default;

        std::optional<RawAudio> LoadFile(const std::filesystem::path& path) override;
        std::optional<RawAudio> LoadMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}