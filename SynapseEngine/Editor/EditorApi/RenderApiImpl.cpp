#include "EditorApiImpl.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Editor/Manager/GuiTextureManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/CameraComponent.h"

#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Rendering/GpuUploader.h"

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

    EntityID EditorApiImpl::ReadEntityIdAtPixel(uint32_t x, uint32_t y)
    {
        auto renderManager = _engine->GetRenderManager();
        if (!renderManager) return NULL_ENTITY;

        auto rtManager = renderManager->GetRenderTargetManager();
        auto frameCtx = ServiceLocator::GetFrameContext();
        uint32_t currentFrame = frameCtx ? frameCtx->currentFrameIndex : 0;

        auto group = rtManager->GetGroup(RenderTargetGroupNames::Deferred, currentFrame);
        if (!group) return NULL_ENTITY;

        auto entityImage = group->GetImage(RenderTargetNames::EntityIndex);
        if (!entityImage) return NULL_ENTITY;

        auto extent = entityImage->GetExtent();
        if (x >= extent.width || y >= extent.height) return NULL_ENTITY;

        Vk::BufferConfig readbackConfig{};
        readbackConfig.size = sizeof(uint32_t);
        readbackConfig.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        readbackConfig.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        readbackConfig.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        readbackConfig.useDeviceAddress = false;

        auto readbackBuffer = Vk::BufferFactory::Create(readbackConfig);

        Vk::GpuUploadRequest request{
            .uploadCallback = [&](VkCommandBuffer cmd) {
                entityImage->TransitionLayout(
                    cmd,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    VK_ACCESS_2_TRANSFER_READ_BIT
                );

                Vk::ImageToBufferCopyInfo copyInfo{};
                copyInfo.srcImage = entityImage->Handle();
                copyInfo.dstBuffer = readbackBuffer->Handle();
                copyInfo.extent = { 1, 1, 1 };
                copyInfo.imageOffset = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };
                copyInfo.bufferOffset = 0;
                copyInfo.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyInfo.srcMipLevel = 0;
                copyInfo.srcBaseLayer = 0;
                copyInfo.layerCount = 1;

                Vk::ImageUtils::CopyImageToBuffer(cmd, copyInfo);

                entityImage->TransitionLayout(
                    cmd,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT
                );
            },
            .needsGraphics = true
        };

        ServiceLocator::GetGpuUploader()->UploadSync(std::move(request));

        EntityID selectedEntity = NULL_ENTITY;
        void* mappedData = readbackBuffer->Map();
        if (mappedData) {
            std::memcpy(&selectedEntity, mappedData, sizeof(uint32_t));
            readbackBuffer->Unmap();
        }

        return selectedEntity;
    }
}