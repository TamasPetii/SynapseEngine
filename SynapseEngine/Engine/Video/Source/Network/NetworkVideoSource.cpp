#include "NetworkVideoSource.h"

namespace Syn
{
    NetworkVideoSource::NetworkVideoSource(const std::string& url, IVideoLoader* loader)
        : _url(url), _loader(loader)
    {
        if (_loader) {
            _state = _loader->OpenNetwork(_url);
        }
    }

    VideoInfo NetworkVideoSource::GetInfo() const
    {
        if (_state) {
            return _state->GetInfo();
        }
        return VideoInfo{};
    }

    std::optional<RawVideoPacket> NetworkVideoSource::ReadNextPacket()
    {
        if (_state) {
            return _state->ReadPacket();
        }
        return std::nullopt;
    }

    void NetworkVideoSource::Reset()
    {
        if (_state) {
            _state->Reset();
        }
    }
}