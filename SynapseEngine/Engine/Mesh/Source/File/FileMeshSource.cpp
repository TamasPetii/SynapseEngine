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
		if (_loader == nullptr) {
            return std::nullopt;
        }

		return _loader->LoadFile(_path);
    }
}