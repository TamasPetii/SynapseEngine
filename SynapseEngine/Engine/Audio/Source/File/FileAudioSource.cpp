#include "FileAudioSource.h"

namespace Syn
{
    std::optional<RawAudio> FileAudioSource::Produce()
    {
        if (!_loader) {
            return std::nullopt;
        }

        return _loader->LoadFile(_path);
    }
}