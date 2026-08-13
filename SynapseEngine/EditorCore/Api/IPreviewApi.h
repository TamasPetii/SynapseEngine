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
#include <cstdint>
#include <imgui.h>
#include <glm/glm.hpp>

#include "IApi.h"
#include "EditorCore/Types/TextureHandle.h"
#include "Engine/Manager/PreviewManager.h" 

namespace Syn {

    struct PreviewItemData {
        uint32_t resourceId;
        glm::vec2 uv0;
        glm::vec2 uv1;
    };

    class IPreviewApi : public IApi {
    public:
        virtual ~IPreviewApi() = default;

        virtual TextureHandle GetAtlasHandle() = 0;
        virtual bool GetPreviewUVs(PreviewResourceType type, uint32_t resourceId, glm::vec2& outUv0, glm::vec2& outUv1) const = 0;
        virtual bool HasPreview(PreviewResourceType type, uint32_t resourceId) const = 0;
        virtual void RequestPreview(PreviewResourceType type, uint32_t resourceId) = 0;
        virtual std::vector<PreviewItemData> GetAllPreviews(PreviewResourceType type) const = 0;
    };
}