// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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