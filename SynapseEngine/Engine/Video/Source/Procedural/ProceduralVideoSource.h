#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Source/IVideoSource.h"

namespace Syn
{
    class SYN_API ProceduralVideoSource : public IVideoSource
    {
    public:
        ProceduralVideoSource(const VideoInfo& info);
        ~ProceduralVideoSource() override = default;

        VideoInfo GetInfo() const override;
        std::optional<RawVideoPacket> ReadNextPacket() override;
        void Reset() override;
    private:
        VideoInfo _info;
        int64_t _currentFrame;
    };
}