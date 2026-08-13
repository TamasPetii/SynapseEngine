// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
        FFmpegCpuVideoConverter(AVCodecID codecId, int width, int height, const std::vector<uint8_t>& extradata);
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