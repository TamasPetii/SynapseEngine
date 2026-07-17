#include "SettingsApiImpl.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Image/ImageManager.h"

namespace Syn {
    SceneSettings SettingsApiImpl::GetSceneSettings() const {
        auto scene = _sceneManager->GetActiveScene();
        return (scene && scene->GetSettings()) ? *(scene->GetSettings()) : SceneSettings{};
    }

    void SettingsApiImpl::SetSceneSettings(const SceneSettings& settings) {
        auto scene = _sceneManager->GetActiveScene();
        if (scene && scene->GetSettings()) {
            *(scene->GetSettings()) = settings;
        }
    }

    std::vector<std::pair<uint32_t, std::string>> SettingsApiImpl::GetAvailableSkyTextures() const {
        std::vector<std::pair<uint32_t, std::string>> result;

        auto imageManager = ServiceLocator::Get<ImageManager>();
        if (!imageManager) return result;

        auto paths = imageManager->GetResourcePaths();
        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (imageManager->GetEntryState(i) == ResourceState::Ready) {
                std::filesystem::path p(paths[i]);
                result.push_back({ i, p.filename().string() });
            }
        }

        return result;
    }
}