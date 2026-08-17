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

#include "RenderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Render/RenderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Manager/PreviewManager.h"
#include <format>

namespace Syn {
    TextureHandle RenderApiImpl::GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) {
        auto renderManager = _engine->GetRenderManager();
        if (!renderManager || renderManager->IsResizePending()) return InvalidTextureHandle;

        auto frameCtx = ServiceLocator::Get<FrameContext>();
        uint32_t currentFrame = frameCtx ? frameCtx->currentFrameIndex : 0;
        std::string cacheKey = std::format("{}_{}_{}_{}", groupName, targetName, viewName, currentFrame);

        if (_viewportTextures.find(cacheKey) == _viewportTextures.end()) {
            if (targetName == RenderTargetNames::DirectionLightShadowDepthPyramid) {
                auto drawData = _sceneManager->GetActiveScene()->GetSceneDrawData();
                auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::NearestClampEdge);
                TextureHandle handle = _textureManager->RegisterTexture(
                    drawData->DirectionLightShadow.shadowDepthPyramid[currentFrame]->GetView(viewName),
                    sampler->Handle()
                );
                _viewportTextures[cacheKey] = handle;
            }
            else if (targetName == RenderTargetNames::SpotLightShadowDepthPyramid) {
                auto drawData = _sceneManager->GetActiveScene()->GetSceneDrawData();
                auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::NearestClampEdge);
                TextureHandle handle = _textureManager->RegisterTexture(
                    drawData->SpotLightShadow.shadowDepthPyramid[currentFrame]->GetView(viewName),
                    sampler->Handle()
                );
                _viewportTextures[cacheKey] = handle;
            }
            else if (targetName == RenderTargetNames::PointLightShadowDepthPyramid) {
                auto drawData = _sceneManager->GetActiveScene()->GetSceneDrawData();
                auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::NearestClampEdge);
                TextureHandle handle = _textureManager->RegisterTexture(
                    drawData->PointLightShadow.shadowDepthPyramid[currentFrame]->GetView(viewName),
                    sampler->Handle()
                );
                _viewportTextures[cacheKey] = handle;
            }
            else if (targetName == RenderTargetNames::PreviewAtlas) {
                auto imageView = ServiceLocator::Get<PreviewManager>()->GetAtlasImage()->GetView(viewName);
                auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
                TextureHandle handle = _textureManager->RegisterTexture(
                    imageView,
                    sampler->Handle()
                );
                _viewportTextures[cacheKey] = handle;
            }
            else {
                auto rtManager = renderManager->GetRenderTargetManager();
                auto group = rtManager->GetGroup(groupName, currentFrame);
                if (!group) return InvalidTextureHandle;
                
                auto image = group->GetImage(targetName);
                if (!image) return InvalidTextureHandle;
                
                auto view = image->GetView(viewName);
                if (!view) return InvalidTextureHandle;

                auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::NearestClampEdge);
                TextureHandle handle = _textureManager->RegisterTexture(image->GetView(viewName), sampler->Handle());
                _viewportTextures[cacheKey] = handle;
            }
        }

        return _textureManager->GetImGuiTextureID(_viewportTextures[cacheKey]);
    }

    void RenderApiImpl::ResizeRenderTargets(uint32_t width, uint32_t height) {
        auto renderManager = _engine->GetRenderManager();
        if (renderManager) {
            renderManager->OnResize(width, height);
            for (auto& pair : _viewportTextures) {
                _textureManager->MarkForDeletion(pair.second);
            }
            _viewportTextures.clear();
        }
    }

    glm::mat4 RenderApiImpl::GetEditorCameraView() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return glm::mat4(1.0f);
        auto settings = scene->GetSettings();
        EntityID cameraEntity = (settings && settings->debug.useDebugCamera) ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, cameraEntity, [](const auto& c) { return c.view; }, glm::mat4(1.0f));
    }

    glm::mat4 RenderApiImpl::GetEditorCameraProjection() const {
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return glm::mat4(1.0f);
        auto settings = scene->GetSettings();
        EntityID cameraEntity = (settings && settings->debug.useDebugCamera) ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        
        return EditorApiUtils::ReadComponent<CameraComponent>(_sceneManager, cameraEntity, [](const auto& c) { return c.proj; }, glm::mat4(1.0f));
    }


    std::pair<EntityID, uint32_t> RenderApiImpl::ReadEntityAndMeshIdAtPixel(uint32_t x, uint32_t y) {
        auto renderManager = _engine->GetRenderManager();
        if (!renderManager) return { NULL_ENTITY, 0 };

        auto rtManager = renderManager->GetRenderTargetManager();
        auto frameCtx = ServiceLocator::Get<FrameContext>();
        uint32_t currentFrame = frameCtx ? frameCtx->currentFrameIndex : 0;

        auto group = rtManager->GetGroup(RenderTargetGroupNames::Main, currentFrame);
        if (!group) return { NULL_ENTITY, 0 };

        auto entityImage = group->GetImage(RenderTargetNames::EntityIndex);
        if (!entityImage) return { NULL_ENTITY, 0 };

        auto extent = entityImage->GetExtent();
        if (x >= extent.width || y >= extent.height) return { NULL_ENTITY, 0 };

        Vk::BufferConfig readbackConfig{};
        readbackConfig.size = sizeof(uint32_t) * 2;
        readbackConfig.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        readbackConfig.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        readbackConfig.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
        readbackConfig.useDeviceAddress = false;

        auto readbackBuffer = Vk::BufferFactory::Create(readbackConfig);

        Vk::GpuUploadRequest request{
            .uploadCallback = [&](VkCommandBuffer cmd, Vk::GpuUploader* gpuUploader) {
                entityImage->TransitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT);
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
                entityImage->TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
            },
			.queueType = Vk::GpuQueueType::Graphics
        };

        ServiceLocator::Get<Vk::GpuUploader>()->UploadSync(std::move(request));

        EntityID selectedEntity = NULL_ENTITY;
        uint32_t selectedMesh = 0;

        void* mappedData = readbackBuffer->Map();
        if (mappedData) {
            uint32_t pixelData[2] = { 0, 0 };
            std::memcpy(pixelData, mappedData, sizeof(uint32_t) * 2);
            readbackBuffer->Unmap();

            uint32_t word0 = pixelData[0];
            selectedEntity = word0 & ~(1u << 31);

            uint32_t word1 = pixelData[1];
            selectedMesh = (word1 >> 22) & 0x3FFu;
        }
        return { selectedEntity, selectedMesh };
    }

    EntityID RenderApiImpl::ReadEntityIdAtPixel(uint32_t x, uint32_t y) {
        return ReadEntityAndMeshIdAtPixel(x, y).first;
    }
}