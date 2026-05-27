#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"
#include "Engine/Scene/Loader/ISceneLoader.h"
#include <filesystem>
#include <memory>

namespace Syn
{
    class SYN_API FileSceneSource : public ISceneSource
    {
    public:
        FileSceneSource(std::filesystem::path path, ISceneLoader* loader);
        virtual bool Populate(Scene& scene) override;
    private:
        std::filesystem::path _path;
        ISceneLoader* _loader;
    };
}