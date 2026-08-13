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
#include <string>
#include <glm/glm.hpp>
#include <cstdint>
#include "EditorCore/Types/TextureHandle.h"
#include "EditorCore/Types/EntityHandle.h"
#include "IApi.h"

namespace Syn {
    class IRenderApi : public IApi {
    public:
        virtual ~IRenderApi() = default;

        virtual void ResizeRenderTargets(uint32_t width, uint32_t height) = 0;
        virtual TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) = 0;
        virtual EntityID ReadEntityIdAtPixel(uint32_t x, uint32_t y) = 0;
        virtual std::pair<EntityID, uint32_t> ReadEntityAndMeshIdAtPixel(uint32_t x, uint32_t y) = 0;

        virtual glm::mat4 GetEditorCameraView() const = 0;
        virtual glm::mat4 GetEditorCameraProjection() const = 0;
    };
}