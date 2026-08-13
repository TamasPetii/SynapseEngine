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
#include "EditorCore/Api/IPreviewApi.h"
#include "Engine/Manager/PreviewManager.h"
#include "Editor/Manager/GuiTextureManager.h"
#include "Engine/Image/ImageManager.h"

namespace Syn {

    class PreviewApiImpl : public IPreviewApi {
    public:
        PreviewApiImpl(PreviewManager* previewManager, GuiTextureManager* guiTextureManager, ImageManager* imageManager);
        ~PreviewApiImpl() override = default;

        TextureHandle GetAtlasHandle() override;
        bool GetPreviewUVs(PreviewResourceType type, uint32_t resourceId, glm::vec2& outUv0, glm::vec2& outUv1) const override;
        bool HasPreview(PreviewResourceType type, uint32_t resourceId) const override;
        void RequestPreview(PreviewResourceType type, uint32_t resourceId) override;
        std::vector<PreviewItemData> GetAllPreviews(PreviewResourceType type) const override;
    private:
        PreviewManager* _previewManager;
        GuiTextureManager* _guiTextureManager;
        ImageManager* _imageManager;

        uint32_t _lastAtlasResolution = 0;
        TextureHandle _atlasTextureHandle;
    };
}