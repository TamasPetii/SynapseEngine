#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Converter/IGpuVideoConverter.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

namespace Syn
{
    class SYN_API FFmpegCpuVideoConverter : public IGpuVideoConverter
    {
    public:
        FFmpegCpuVideoConverter(AVCodecID codecId, int width, int height);
        ~FFmpegCpuVideoConverter() override;

        GpuVideoPacket Convert(const CookedVideoPacket& cookedPacket) override;

    private:
        AVCodecContext* _codecContext = nullptr;
        AVFrame* _frame = nullptr;
        AVFrame* _rgbFrame = nullptr;
        SwsContext* _swsContext = nullptr;

        int _width = 0;
        int _height = 0;
    };
}