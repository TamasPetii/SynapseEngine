#include "CompositePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {

    void CompositePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("CompositeProgram", {
            ShaderNames::CompositeVert,
            ShaderNames::CompositeFrag
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
                .testEnable = VK_FALSE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_ALWAYS
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

    void CompositePass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto inputImage = group->GetImage(RenderTargetNames::Main);
        auto swapchainImage = swapChain->GetImage(context.swapchainImageIndex);

        VkExtent2D extent = { swapchainImage->GetExtent().width, swapchainImage->GetExtent().height };
        _graphicsState.renderArea = extent;

        _imageTransitions.push_back({
            .image = inputImage,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = swapchainImage,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = swapchainImage->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = nullptr,
            .layerCount = 1
        };
    }

    void CompositePass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto inputImage = group->GetImage(RenderTargetNames::Main);
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            inputImage->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void CompositePass::Draw(const RenderContext& context) {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}