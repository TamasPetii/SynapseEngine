#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Source/IVideoSource.h"
#include "Engine/Video/Loader/IVideoLoader.h"
#include <filesystem>
#include <memory>

namespace Syn
{
    class SYN_API FileVideoSource : public IVideoSource
    {
    public:
        FileVideoSource(const std::filesystem::path& path, IVideoLoader* loader);
        ~FileVideoSource() override = default;

        VideoInfo GetInfo() const override;
        std::optional<RawVideoPacket> ReadNextPacket() override;
        void Reset() override;
    private:
        std::filesystem::path _path;
        IVideoLoader* _loader;
        std::unique_ptr<IVideoState> _state;
    };
}