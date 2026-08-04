#pragma once
#include "../IAudioSource.h"
#include "Engine/Audio/Loader/IAudioLoader.h"
#include <filesystem>

namespace Syn
{
    class SYN_API FileAudioSource : public IAudioSource
    {
    public:
        FileAudioSource(const std::filesystem::path& path, IAudioLoader* loader)
            : _path(path), _loader(loader) {}

        std::optional<RawAudio> Produce() override;
    private:
        std::filesystem::path _path;
        IAudioLoader* _loader;
    };
}