#include "AnimationPreviewPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Manager/PreviewManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/FrameContext.h" 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/AnimationPreviewPC.glsl"

    void AnimationPreviewPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgramId = shaderManager->LoadProgramAsync("AnimationPreviewProgram", {
            ShaderNames::AnimationPreviewVert,
            ShaderNames::AnimationPreviewFrag,
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blendStates = {
                {
                    .enable = VK_FALSE,
                    .srcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void AnimationPreviewPass::PrepareFrame(const RenderContext& context) {
        _dirtyAnimations.clear();

        auto pm = ServiceLocator::Get<PreviewManager>();

        _dirtyAnimations = pm->GetDirtyResources(PreviewResourceType::Animation);
        if (_dirtyAnimations.empty()) return;

        auto atlas = pm->GetAtlasImage();
        auto atlasDepth = pm->GetAtlasDepthImage();
        VkExtent2D extent = { atlas->GetExtent().width, atlas->GetExtent().height };
        _graphicsState.renderArea = std::nullopt;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = atlas->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = atlasDepth->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{.depthStencil = {1.0f, 0}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void AnimationPreviewPass::BindDescriptors(const RenderContext& context) {
        if (_dirtyAnimations.empty()) return;

        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void AnimationPreviewPass::Draw(const RenderContext& context) {
        if (_dirtyAnimations.empty() || !_renderInfo.has_value()) return;

        auto pm = ServiceLocator::Get<PreviewManager>();
        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto animManager = ServiceLocator::Get<AnimationManager>();
        auto frameCtx = ServiceLocator::Get<FrameContext>();

        static float previewTime = 0.0f;
        if (frameCtx) {
            previewTime += frameCtx->deltaTime;
        }

        Vk::PushConstant<AnimationPreviewPC> pc;
        pc->frameGlobalContextBufferAddr = context.scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);

        auto animSnapshot = animManager->GetResourceSnapshot();
        auto modelSnapshot = modelManager->GetResourceSnapshot();

        for (uint32_t animId : _dirtyAnimations)
        {
            auto animResource = animSnapshot[animId].resource;
            if (animResource == nullptr) continue;

            uint32_t baseModelId = animResource->cpuData.baseModelId;
            if (baseModelId >= modelSnapshot.size() || modelSnapshot[baseModelId].resource == nullptr) continue;

            auto modelResource = modelSnapshot[baseModelId].resource;

            VkViewport viewport{};
            VkRect2D scissor{};
            pm->GetViewportAndScissor(PreviewResourceType::Animation, animId, viewport, scissor);

            vkCmdSetViewportWithCount(context.cmd, 1, &viewport);
            vkCmdSetScissorWithCount(context.cmd, 1, &scissor);

            VkClearAttachment clearAttachments[2] = {};

            clearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clearAttachments[0].colorAttachment = 0;
            clearAttachments[0].clearValue.color = { {0.15f, 0.15f, 0.15f, 1.0f} };

            clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            clearAttachments[1].clearValue.depthStencil = { 1.0f, 0 };

            VkClearRect clearRect{};
            clearRect.rect = scissor;
            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = 1;
            vkCmdClearAttachments(context.cmd, 2, clearAttachments, 1, &clearRect);

            const auto& animCpuData = animResource->cpuData;

			auto globalCollider = animCpuData.globalCollider;
			globalCollider = modelResource->cpuData.globalCollider;

            glm::vec3 center = globalCollider.center;
            float radius = globalCollider.radius > 0.0f ? globalCollider.radius * 1.05f : 1.0f;

            float targetSize = 10.0f;
            float scaleFactor = targetSize / (radius * 2.0f);
            float cameraDistance = targetSize * 1.25f;

            glm::vec3 camDir = glm::normalize(glm::vec3(1.0f, 0.75f, 1.0f));
            glm::vec3 camPos = camDir * cameraDistance;
            glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            float nearPlane = targetSize * 0.05f;
            float farPlane = targetSize * 5.0f;
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, nearPlane, farPlane);
            proj[1][1] *= -1.0f;

            glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));
            model = glm::translate(model, -center);

            pc->modelId = baseModelId;
            pc->animationId = animId;

            float duration = animCpuData.descriptor.durationInSeconds;
            uint32_t maxFrames = animCpuData.descriptor.frameCount;

            if (duration > 0.0f && maxFrames > 0) {
                float time = std::fmod(previewTime, duration);
                float progress = time / duration;
                pc->frameIndex = static_cast<uint32_t>(progress * maxFrames);

                if (pc->frameIndex >= maxFrames) {
                    pc->frameIndex = maxFrames - 1;
                }
            }
            else {
                pc->frameIndex = 0;
            }

            pc->mvp = proj * view * model;

            const auto& cpuData = modelResource->cpuData;
            const auto& drawCommands = cpuData.baseDrawCommands;
            uint32_t meshCount = cpuData.globalMeshCount;

            for (uint32_t meshIdx = 0; meshIdx < meshCount; ++meshIdx) {
                pc->meshIndex = meshIdx;
                pc.Push(context.cmd, _shaderProgram->GetLayout());

                uint32_t lod0CommandIndex = meshIdx * 4;
                const auto& cmd = drawCommands[lod0CommandIndex];

                vkCmdDraw(context.cmd, cmd.traditionalCmd.vertexCount, 1, cmd.traditionalCmd.firstVertex, cmd.traditionalCmd.firstInstance);
            }
        }
    }
}