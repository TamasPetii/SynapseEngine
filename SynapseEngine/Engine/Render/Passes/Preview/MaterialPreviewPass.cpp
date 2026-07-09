#include "MaterialPreviewPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Manager/PreviewManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/MaterialPreviewPC.glsl"

    void MaterialPreviewPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        auto imageManager = ServiceLocator::GetImageManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("MaterialPreviewProgram", {
            ShaderNames::MaterialPreviewVert,
            ShaderNames::MaterialPreviewFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_FALSE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS
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

    void MaterialPreviewPass::PrepareFrame(const RenderContext& context) {
        _dirtyMaterials.clear();
        _renderInfo.reset();

        auto pm = ServiceLocator::GetPreviewManager();
        auto modelManager = ServiceLocator::GetModelManager();

        _dirtyMaterials = pm->GetDirtyResources(PreviewResourceType::Material);
        if (_dirtyMaterials.empty()) return;

        auto atlas = pm->GetAtlasImage();
        VkExtent2D extent = { atlas->GetExtent().width, atlas->GetExtent().height };
        _graphicsState.renderArea = std::nullopt;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = atlas->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .layerCount = 1
        };
    }

    void MaterialPreviewPass::BindDescriptors(const RenderContext& context) {
        if (_dirtyMaterials.empty()) return;

        auto imageManager = ServiceLocator::GetImageManager();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void MaterialPreviewPass::Draw(const RenderContext& context) {
        if (_dirtyMaterials.empty() || !_renderInfo.has_value()) return;

        auto pm = ServiceLocator::GetPreviewManager();
        auto modelManager = ServiceLocator::GetModelManager();

        Vk::PushConstant<MaterialPreviewPC> pc;
        pc->frameGlobalContextBufferAddr = context.scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);

        for (uint32_t matId : _dirtyMaterials) {
            VkViewport viewport{};
            VkRect2D scissor{};
            pm->GetViewportAndScissor(PreviewResourceType::Material, matId, viewport, scissor);

            vkCmdSetViewportWithCount(context.cmd, 1, &viewport);
            vkCmdSetScissorWithCount(context.cmd, 1, &scissor);

            VkClearAttachment clearAttachment{};
            clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clearAttachment.colorAttachment = 0;
            clearAttachment.clearValue.color = { {0.15f, 0.15f, 0.15f, 1.0f} };

            VkClearRect clearRect{};
            clearRect.rect = scissor;
            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = 1;

            vkCmdClearAttachments(context.cmd, 1, &clearAttachment, 1, &clearRect);

            pc->materialId = matId;
            pc.Push(context.cmd, _shaderProgram->GetLayout());

            vkCmdDraw(context.cmd, 3, 1, 0, 0);
        }
    }
}