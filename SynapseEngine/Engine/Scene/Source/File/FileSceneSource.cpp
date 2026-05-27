#include "FileSceneSource.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    FileSceneSource::FileSceneSource(std::filesystem::path path, ISceneLoader* loader)
        : _path(std::move(path)), _loader(loader)
    {}

    bool FileSceneSource::Populate(Scene& scene)
    {
        if (!_loader)
        {
            Error("FileSceneSource: No loader provided!");
            return false;
        }

        return _loader->LoadScene(scene, _path);
    }
}