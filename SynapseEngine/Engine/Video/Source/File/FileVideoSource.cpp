#include "FileVideoSource.h"

namespace Syn
{
    FileVideoSource::FileVideoSource(const std::filesystem::path& path, IVideoLoader* loader)
        : _path(path), _loader(loader)
    {
        if (_loader) {
            _state = _loader->OpenFile(_path);
        }
    }

    VideoInfo FileVideoSource::GetInfo() const
    {
        if (_state) {
            return _state->GetInfo();
        }
        return VideoInfo{};
    }

    std::optional<RawVideoPacket> FileVideoSource::ReadNextPacket()
    {
        if (_state) {
            return _state->ReadPacket();
        }
        return std::nullopt;
    }

    void FileVideoSource::Reset()
    {
        if (_state) {
            _state->Reset();
        }
    }
}