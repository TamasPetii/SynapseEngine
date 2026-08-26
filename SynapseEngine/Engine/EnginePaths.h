#pragma once
#include "Engine/SynApi.h"
#include <filesystem>

namespace Syn
{
    struct SYN_API EnginePaths
    {
        static std::filesystem::path GetAppDataDir();
        static std::filesystem::path GetBaseDir();
        static std::filesystem::path GetCacheDir();
        static std::filesystem::path GetImagesCacheDir();
        static std::filesystem::path GetModelsCacheDir();
        static std::filesystem::path GetAnimationsCacheDir();
        static std::filesystem::path GetShadersCacheDir();
        static std::filesystem::path GetScenesCacheDir();
        static std::filesystem::path GetLogsDir();
        static std::filesystem::path GetTestSavesDir();
    private:
        static std::filesystem::path InternalGetOrCreateDir(const std::filesystem::path& basePath, const std::string& folderName);
    };
}