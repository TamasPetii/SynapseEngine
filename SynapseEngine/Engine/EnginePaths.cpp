#include "EnginePaths.h"
#include <cstdlib>

namespace Syn
{
    std::filesystem::path EnginePaths::InternalGetOrCreateDir(const std::filesystem::path& basePath, const std::string& folderName) {
        std::filesystem::path dir = basePath / folderName;
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
        return dir;
    }

    std::filesystem::path EnginePaths::GetAppDataDir() {
        const char* appDataPath = std::getenv("APPDATA");
        return appDataPath ? std::filesystem::path(appDataPath) : std::filesystem::path(".");
    }

    std::filesystem::path EnginePaths::GetBaseDir() {
        return GetAppDataDir() / "Synapse";
    }

    std::filesystem::path EnginePaths::GetCacheDir() {
        return GetBaseDir() / "Cache";
    }

    std::filesystem::path EnginePaths::GetImagesCacheDir() {
        return InternalGetOrCreateDir(GetCacheDir(), "Images");
    }

    std::filesystem::path EnginePaths::GetModelsCacheDir() {
        return InternalGetOrCreateDir(GetCacheDir(), "Models");
    }

    std::filesystem::path EnginePaths::GetAnimationsCacheDir() {
        return InternalGetOrCreateDir(GetCacheDir(), "Animations");
    }

    std::filesystem::path EnginePaths::GetScenesCacheDir() {
        return InternalGetOrCreateDir(GetCacheDir(), "Scenes");
    }

    std::filesystem::path EnginePaths::GetShadersCacheDir() {
        return InternalGetOrCreateDir(GetBaseDir(), "Shaders");
    }

    std::filesystem::path EnginePaths::GetLogsDir() {
        return InternalGetOrCreateDir(GetBaseDir(), "Logs");
    }

    std::filesystem::path EnginePaths::GetTestSavesDir() {
        return InternalGetOrCreateDir(GetBaseDir(), "TestSaves");
    }
}