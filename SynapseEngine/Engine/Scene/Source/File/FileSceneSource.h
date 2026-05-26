#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"
#include <filesystem>

namespace Syn
{
    class SYN_API FileSceneSource : public ISceneSource
    {
    public:
        FileSceneSource(std::filesystem::path path);
        virtual bool Populate(Scene& scene) override;
    private:
        std::filesystem::path _path;
    };
}