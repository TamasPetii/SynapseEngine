#include "FileAnimationSource.h"
#include "Engine/Animation/Loader/IAnimationLoader.h"

namespace Syn
{
    FileAnimationSource::FileAnimationSource(const std::filesystem::path& path, IAnimationLoader* loader)
        : _path(path), _loader(loader)
    {}

    std::optional<RawAnimation> FileAnimationSource::Produce()
    {
        if (_loader == nullptr) {
            return std::nullopt;
        }

        return _loader->LoadFile(_path);
    }
}