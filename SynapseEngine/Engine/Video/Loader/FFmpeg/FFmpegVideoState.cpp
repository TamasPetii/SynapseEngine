#include "FFmpegVideoState.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    FFmpegVideoState::FFmpegVideoState(const std::string& path)
    {
        if (avformat_open_input(&_formatContext, path.c_str(), nullptr, nullptr) != 0) {
            Error("FFmpeg failed to open file: {}", path);
            return;
        }

        if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
            Error("FFmpeg failed to find stream info: {}", path);
            return;
        }

        for (unsigned int i = 0; i < _formatContext->nb_streams; i++) {
            if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                _videoStreamIndex = i;
                break;
            }
        }

        if (_videoStreamIndex != -1) {
            AVStream* stream = _formatContext->streams[_videoStreamIndex];
            _info.width = stream->codecpar->width;
            _info.height = stream->codecpar->height;
            _info.duration = _formatContext->duration;

            if (stream->avg_frame_rate.den > 0) {
                _info.frameRate = av_q2d(stream->avg_frame_rate);
            }
        }
    }

    FFmpegVideoState::~FFmpegVideoState()
    {
        if (_formatContext) {
            avformat_close_input(&_formatContext);
        }
    }

    VideoInfo FFmpegVideoState::GetInfo() const
    {
        return _info;
    }

    std::optional<RawVideoPacket> FFmpegVideoState::ReadPacket()
    {
        if (!_formatContext || _videoStreamIndex == -1) {
            return std::nullopt;
        }

        AVPacket* packet = av_packet_alloc();
        while (av_read_frame(_formatContext, packet) >= 0) {
            if (packet->stream_index == _videoStreamIndex) {
                RawVideoPacket rawPacket;
                rawPacket.data.assign(packet->data, packet->data + packet->size);
                rawPacket.pts = packet->pts;
                rawPacket.dts = packet->dts;
                rawPacket.isKeyFrame = (packet->flags & AV_PKT_FLAG_KEY);

                av_packet_unref(packet);
                av_packet_free(&packet);
                return rawPacket;
            }
            av_packet_unref(packet);
        }

        av_packet_free(&packet);
        return std::nullopt;
    }

    void FFmpegVideoState::Reset()
    {
        if (_formatContext && _videoStreamIndex != -1) {
            av_seek_frame(_formatContext, _videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
        }
    }
}