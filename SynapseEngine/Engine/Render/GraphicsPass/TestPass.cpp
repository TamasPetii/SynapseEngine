#include "TestPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"

namespace Syn {
    void TestPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("TestColorProgram", {
            "../Engine/Shaders/Fullscreen.vert",
            "../Engine/Shaders/Fullscreen.frag"
            });

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = false,
                .writeEnable = false,
                .compareOp = VK_COMPARE_OP_ALWAYS
            },
            .blend = {
                .enable = false,
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

    void TestPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto image = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = swapChain->GetExtent();

        _graphicsState.renderArea = extent;

        Vk::AttachmentConfig colorConfig = {
            .imageView = image->GetView("_default"),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .clearValue = VkClearValue{{{0.1f, 0.1f, 0.1f, 1.0f}}},
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        _colorAttachments = { Vk::RenderUtils::CreateAttachment(colorConfig) };

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = nullptr,
            .stencilAttachment = nullptr,
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
    }

    void TestPass::Draw(const RenderContext& context) 
    {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}