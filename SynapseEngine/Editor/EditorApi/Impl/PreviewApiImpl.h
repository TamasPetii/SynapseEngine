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