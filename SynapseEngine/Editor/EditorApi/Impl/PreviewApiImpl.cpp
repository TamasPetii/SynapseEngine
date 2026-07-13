#include "PreviewApiImpl.h"
#include "Engine/Image/SamplerNames.h"

namespace Syn {

    PreviewApiImpl::PreviewApiImpl(PreviewManager* previewManager, GuiTextureManager* guiTextureManager, ImageManager* imageManager)
        : _previewManager(previewManager), _guiTextureManager(guiTextureManager), _imageManager(imageManager)
    {}

    TextureHandle PreviewApiImpl::GetAtlasHandle() {
        if (!_previewManager || !_guiTextureManager || !_imageManager) 
            return InvalidTextureHandle;

        auto* atlasImage = _previewManager->GetAtlasImage();
        if (!atlasImage) 
            return InvalidTextureHandle;

        uint32_t currentRes = _previewManager->GetResolution();

        if (_lastAtlasResolution != currentRes) {
            auto sampler = _imageManager->GetSampler(SamplerNames::LinearClampEdge);
            _atlasTextureHandle = _guiTextureManager->RegisterTexture(
                atlasImage->GetView(),
                sampler->Handle()
            );

            _lastAtlasResolution = currentRes;
        }

        return _guiTextureManager->GetImGuiTextureID(_atlasTextureHandle);
    }

    bool PreviewApiImpl::GetPreviewUVs(PreviewResourceType type, uint32_t resourceId, glm::vec2& outUv0, glm::vec2& outUv1) const {
        if (!_previewManager) return false;

        if (_previewManager->HasTile(type, resourceId)) {
            _previewManager->GetNormalizedUVs(type, resourceId, outUv0, outUv1);
            return true;
        }

        outUv0 = glm::vec2(0.0f);
        outUv1 = glm::vec2(1.0f);
        return false;
    }

    bool PreviewApiImpl::HasPreview(PreviewResourceType type, uint32_t resourceId) const {
        if (!_previewManager) return false;
        return _previewManager->HasTile(type, resourceId);
    }

    void PreviewApiImpl::RequestPreview(PreviewResourceType type, uint32_t resourceId) {
        if (!_previewManager) return;
        _previewManager->AllocateTile(type, resourceId);
        _previewManager->MarkDirty(type, resourceId);
    }

    std::vector<PreviewItemData> PreviewApiImpl::GetAllPreviews(PreviewResourceType type) const {
        if (!_previewManager) return {};

        std::vector<PreviewItemData> result;
        auto activeIds = _previewManager->GetActiveResources(type);
        result.reserve(activeIds.size());

        for (uint32_t id : activeIds) {
            glm::vec2 uv0, uv1;
            _previewManager->GetNormalizedUVs(type, id, uv0, uv1);
            result.push_back({ id, uv0, uv1 });
        }

        return result;
    }
}