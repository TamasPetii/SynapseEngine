#pragma once
#include "Engine/SynApi.h"
#include <filesystem>

namespace Syn
{
    class Scene;

    class SYN_API ISceneLoader
    {
    public:
        virtual ~ISceneLoader() = default;
        virtual bool LoadScene(Scene& scene, const std::filesystem::path& path) = 0;
    };
}