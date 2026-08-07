#include "ProceduralVideoSource.h"

namespace Syn
{
    ProceduralVideoSource::ProceduralVideoSource(const VideoInfo& info)
        : _info(info), _currentFrame(0)
    {}

    VideoInfo ProceduralVideoSource::GetInfo() const
    {
        return _info;
    }

    std::optional<RawVideoPacket> ProceduralVideoSource::ReadNextPacket()
    {
        if (_info.duration > 0 && _currentFrame >= _info.duration) {
            return std::nullopt;
        }

        RawVideoPacket packet;
        packet.pts = _currentFrame;
        packet.dts = _currentFrame;
        packet.isKeyFrame = (_currentFrame % 30 == 0);
        packet.data.resize(1024, 0);

        _currentFrame++;
        return packet;
    }

    void ProceduralVideoSource::Reset()
    {
        _currentFrame = 0;
    }
}