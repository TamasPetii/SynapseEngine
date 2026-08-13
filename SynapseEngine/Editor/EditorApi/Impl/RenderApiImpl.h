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

#pragma once
#include "EditorCore/Api/IRenderApi.h"
#include "Engine/Engine.h"
#include "Engine/Scene/SceneManager.h"
#include "Editor/Manager/GuiTextureManager.h"
#include <unordered_map>

namespace Syn {
    class RenderApiImpl : public IRenderApi {
    public:
        RenderApiImpl(Engine* engine, GuiTextureManager* textureManager, SceneManager* sm) 
            : _engine(engine), _textureManager(textureManager), _sceneManager(sm) {}

        void ResizeRenderTargets(uint32_t width, uint32_t height) override;
        TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) override;
        EntityID ReadEntityIdAtPixel(uint32_t x, uint32_t y) override;
        std::pair<EntityID, uint32_t> ReadEntityAndMeshIdAtPixel(uint32_t x, uint32_t y) override;
        glm::mat4 GetEditorCameraView() const override;
        glm::mat4 GetEditorCameraProjection() const override;
    private:
        Engine* _engine;
        GuiTextureManager* _textureManager;
        SceneManager* _sceneManager;
        std::unordered_map<std::string, TextureHandle> _viewportTextures;
    };
}