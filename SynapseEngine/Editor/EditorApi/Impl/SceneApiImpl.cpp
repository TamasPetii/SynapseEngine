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

#include "SceneApiImpl.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/EnginePaths.h"
#include <filesystem>

namespace Syn 
{
    void SceneApiImpl::NewScene() {
        Syn::Info("SceneApiImpl: New Scene intent triggered.");
    }

    void SceneApiImpl::LoadScene(const std::string& filepath) {
        std::filesystem::path loadPath = filepath.empty() ? EnginePaths::GetScenesCacheDir() / "Temp.synscene" : std::filesystem::path(filepath);
        _sceneManager->LoadSceneFromFile(loadPath.string());
        Syn::Info("SceneApiImpl: Scene loaded from {}", loadPath.string());
    }

    void SceneApiImpl::SaveScene(const std::string& filepath) {
        if (!_sceneManager->GetActiveScene()) return;
        std::filesystem::path savePath = filepath.empty() ? EnginePaths::GetScenesCacheDir() / "Temp.synscene" : std::filesystem::path(filepath);
        _sceneManager->SaveActiveScene(savePath.string());
        Syn::Info("SceneApiImpl: Scene saved to {}", savePath.string());
    }

    void SceneApiImpl::ActivateScene(const std::string& sceneName) {
        if (_sceneManager) {
            _sceneManager->LoadScene(sceneName);
            Syn::Info("SceneApiImpl: Activated registered scene: {}", sceneName);
        }
    }
}