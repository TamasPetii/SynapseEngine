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

#include "FFmpegCpuVideoConverter.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    FFmpegCpuVideoConverter::FFmpegCpuVideoConverter(AVCodecID codecId, int width, int height, const std::vector<uint8_t>& extradata)
        : _width(width), _height(height)
    {
        const AVCodec* codec = avcodec_find_decoder(codecId);
        _codecContext = avcodec_alloc_context3(codec);

        _codecContext->width = width;
        _codecContext->height = height;

        if (!extradata.empty()) {
            _codecContext->extradata_size = static_cast<int>(extradata.size());
            _codecContext->extradata = static_cast<uint8_t*>(av_mallocz(extradata.size() + AV_INPUT_BUFFER_PADDING_SIZE));
            std::memcpy(_codecContext->extradata, extradata.data(), extradata.size());
        }

        avcodec_open2(_codecContext, codec, nullptr);

        _frame = av_frame_alloc();
        _rgbFrame = av_frame_alloc();

        _rgbFrame->format = AV_PIX_FMT_RGBA;
        _rgbFrame->width = width;
        _rgbFrame->height = height;
        av_frame_get_buffer(_rgbFrame, 0);

        _swsContext = sws_getContext(
            width, height, AV_PIX_FMT_YUV420P,
            width, height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    FFmpegCpuVideoConverter::~FFmpegCpuVideoConverter()
    {
        if (_swsContext) sws_freeContext(_swsContext);
        if (_rgbFrame) av_frame_free(&_rgbFrame);
        if (_frame) av_frame_free(&_frame);
        if (_codecContext) {
            avcodec_free_context(&_codecContext);
        }
    }

    GpuVideoPacket FFmpegCpuVideoConverter::Convert(const CookedVideoPacket& cookedPacket)
    {
        GpuVideoPacket gpuPacket{};
        gpuPacket.pts = cookedPacket.pts;
        gpuPacket.dts = cookedPacket.dts;
        gpuPacket.isKeyFrame = cookedPacket.isKeyFrame;
        gpuPacket.format = VK_FORMAT_R8G8B8A8_UNORM;

        AVPacket* packet = av_packet_alloc();
        packet->data = const_cast<uint8_t*>(cookedPacket.data.data());
        packet->size = static_cast<int>(cookedPacket.data.size());

        if (avcodec_send_packet(_codecContext, packet) == 0) {
            if (avcodec_receive_frame(_codecContext, _frame) == 0) {
                sws_scale(
                    _swsContext,
                    _frame->data, _frame->linesize,
                    0, _codecContext->height,
                    _rgbFrame->data, _rgbFrame->linesize
                );

                size_t dataSize = _width * _height * 4;
                gpuPacket.bitstreamData.assign(_rgbFrame->data[0], _rgbFrame->data[0] + dataSize);
            }
        }

        av_packet_free(&packet);
        return gpuPacket;
    }
}