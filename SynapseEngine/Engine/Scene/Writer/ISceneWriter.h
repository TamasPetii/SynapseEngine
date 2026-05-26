#pragma once
#include "Engine/SynApi.h"
#include <filesystem>

namespace Syn
{
    class Scene;

    class SYN_API ISceneWriter
    {
    public:
        virtual ~ISceneWriter() = default;
        virtual bool SaveScene(Scene& scene, const std::filesystem::path& path) = 0;
    };
}