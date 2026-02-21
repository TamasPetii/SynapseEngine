#include "FileMeshSource.h"
#include "Engine/Mesh/Loader/IMeshLoader.h"

namespace Syn
{
    FileMeshSource::FileMeshSource(const std::filesystem::path& path, IMeshLoader* loader)
        : _path(path), _loader(loader)
    {
    }

    std::optional<RawModel> FileMeshSource::Produce()
    {
		return _loader->LoadFile(_path);
    }
}