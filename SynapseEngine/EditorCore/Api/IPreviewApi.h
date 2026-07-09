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