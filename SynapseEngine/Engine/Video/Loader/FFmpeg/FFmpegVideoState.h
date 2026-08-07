#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Loader/IVideoState.h"
#include <string>

extern "C" {
    #include <libavformat/avformat.h>
}

namespace Syn
{
    class SYN_API FFmpegVideoState : public IVideoState
    {
    public:
        FFmpegVideoState(const std::string& path);
        ~FFmpegVideoState() override;

        VideoInfo GetInfo() const override;
        std::optional<RawVideoPacket> ReadPacket() override;
        void Reset() override;
    private:
        AVFormatContext* _formatContext = nullptr;
        int _videoStreamIndex = -1;
        VideoInfo _info{};
    };
}