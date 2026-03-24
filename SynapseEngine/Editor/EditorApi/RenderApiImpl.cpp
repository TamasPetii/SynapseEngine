#include "EditorApiImpl.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Editor/Manager/GuiTextureManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/CameraComponent.h"
#include <format>

namespace Syn 
{
    TextureHandle EditorApiImpl::GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) {
        auto renderManager = _engine->GetRenderManager();

        if (!renderManager || renderManager->IsResizePending()) {
            return InvalidTextureHandle;
        }

        auto frameCtx = ServiceLocator::GetFrameContext();
        uint32_t currentFrame = frameCtx ? frameCtx->currentFrameIndex : 0;
        std::string cacheKey = std::format("{}_{}_{}_{}", groupName, targetName, viewName, currentFrame);

        if (_viewportTextures.find(cacheKey) == _viewportTextures.end()) {
            auto rtManager = renderManager->GetRenderTargetManager();

            auto group = rtManager->GetGroup(groupName, currentFrame);
            if (!group) return InvalidTextureHandle;

            auto image = group->GetImage(targetName);
            if (!image) return InvalidTextureHandle;

            auto view = image->GetView(viewName);
            if (!view) return InvalidTextureHandle;

            auto sampler = ServiceLocator::GetImageManager()->GetSampler(SamplerNames::NearestClampEdge);
            TextureHandle handle = GuiTextureManager::Get().RegisterTexture(image->GetView(viewName), sampler->Handle());
            _viewportTextures[cacheKey] = handle;
        }

        return GuiTextureManager::Get().GetImGuiTextureID(_viewportTextures[cacheKey]);
    }

    void EditorApiImpl::ResizeRenderTargets(uint32_t width, uint32_t height) {
        auto renderManager = _engine->GetRenderManager();
        if (renderManager) 
        {
            renderManager->OnResize(width, height);

            for (auto& pair : _viewportTextures) {
                GuiTextureManager::Get().MarkForDeletion(pair.second);
            }

            _viewportTextures.clear();
        }
    }

    glm::mat4 EditorApiImpl::GetEditorCameraView() const {
        constexpr auto nullValue = glm::mat4(1.0f);
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return nullValue;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return nullValue;

        auto cameraEntity = scene->GetSceneCameraEntity();
        if (cameraEntity == NULL_ENTITY || !registry->HasComponent<CameraComponent>(cameraEntity))
            return nullValue;

        return registry->GetComponent<CameraComponent>(cameraEntity).view;
    }

    glm::mat4 EditorApiImpl::GetEditorCameraProjection() const {
        constexpr auto nullValue = glm::mat4(1.0f);
        auto scene = _sceneManager->GetActiveScene();
        if (scene == nullptr) return nullValue;

        auto registry = scene->GetRegistry();
        if (registry == nullptr) return nullValue;

        auto cameraEntity = scene->GetSceneCameraEntity();
        if (cameraEntity == NULL_ENTITY || !registry->HasComponent<CameraComponent>(cameraEntity))
            return nullValue;

        return registry->GetComponent<CameraComponent>(cameraEntity).proj;
    }
}