#include "MeshletRenderPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/System/RenderSystem.h"

namespace Syn
{
    struct MeshletPushConstants {

    };

    void MeshletRenderPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("MeshletColorProgram", {
                "../Engine/Shaders/Meshlet.mesh",
                "../Engine/Shaders/Meshlet.frag"
            });

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
            .blend = {
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void MeshletRenderPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto image = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = swapChain->GetExtent();

        _graphicsState.renderArea = extent;

        static std::unique_ptr<Vk::Image> depthImage = nullptr;

        if (!depthImage || depthImage->GetExtent().width != extent.width || depthImage->GetExtent().height != extent.height) {
            depthImage = Vk::ImageFactory::CreateAttachment(
                extent.width,
                extent.height,
                VK_FORMAT_D32_SFLOAT,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }

        Vk::AttachmentConfig colorConfig = {
            .imageView = image->GetView("_default"),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{{{0.1f, 0.1f, 0.1f, 1.0f}}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        Vk::AttachmentConfig depthConfig = {
            .imageView = depthImage->GetView("_default"),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{.depthStencil = {1.0f, 0}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        _colorAttachments = { Vk::RenderUtils::CreateAttachment(colorConfig) };
        _depthAttachment = Vk::RenderUtils::CreateAttachment(depthConfig);

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = _depthAttachment.has_value() ? &_depthAttachment.value() : nullptr,
            .stencilAttachment = _stencilAttachment.has_value() ? &_stencilAttachment.value() : nullptr,
            .layerCount = 1
        };

        _imageTransitions.clear();

        _imageTransitions.push_back({
            .image = image,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });

        _imageTransitions.push_back({
            .image = depthImage.get(),
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });
    }

    void MeshletRenderPass::PushConstants(const RenderContext& context) {

    }

    void MeshletRenderPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->globalIndirectCommandBuffers[context.frameIndex]->Handle();
        auto countBuffer = drawData->globalDrawCountBuffers[context.frameIndex]->Handle();

        VkDeviceSize indirectOffset = SceneDrawData::MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        VkDeviceSize countOffset = sizeof(uint32_t);

        vkCmdDrawMeshTasksIndirectCountEXT(
            context.cmd,
            indirectBuffer,
            indirectOffset,
            countBuffer,
            countOffset,
            SceneDrawData::MAX_INDIRECT_COMMANDS,
            sizeof(VkDrawMeshTasksIndirectCommandEXT)
        );
    }
}