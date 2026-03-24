#pragma once
#include <string>
#include <glm/glm.hpp>
#include <cstdint>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    class IRenderAPI {
    public:
        virtual ~IRenderAPI() = default;

        virtual void ResizeRenderTargets(uint32_t width, uint32_t height) = 0;
        virtual TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) = 0;
    
        virtual glm::mat4 GetEditorCameraView() const = 0;
        virtual glm::mat4 GetEditorCameraProjection() const = 0;
    };
}