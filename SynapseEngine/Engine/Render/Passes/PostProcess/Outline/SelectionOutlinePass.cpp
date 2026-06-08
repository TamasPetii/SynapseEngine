#include "SelectionOutlinePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn
{
    #include "Engine/Shaders/Includes/PushConstants/SelectionOutlinePC.glsl"

    bool SelectionOutlinePass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableSelectedOutline && context.scene->GetSelectedEntity() != NULL_ENTITY;
    }

    void SelectionOutlinePass::Initialize()
    {
        auto imageManager = ServiceLocator::GetImageManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = ServiceLocator::GetShaderManager()->CreateProgram("SelectionOutlineProgram", {
            ShaderNames::FullscreenVert,
            ShaderNames::SelectionOutlineFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .polygonMode = VK_POLYGON_MODE_FILL,
            },
            .depth = {
                .testEnable = VK_FALSE,
                .writeEnable = VK_FALSE,
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 1
        };
    }

    void SelectionOutlinePass::PrepareFrame(const RenderContext& context)
    {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        _graphicsState.renderArea = VkExtent2D{ group->GetWidth(), group->GetHeight() };

        auto mainImage = group->GetImage(RenderTargetNames::Main);
        auto entityImage = group->GetImage(RenderTargetNames::EntityIndex);
        auto depthImage = group->GetImage(RenderTargetNames::DepthPyramid);

        _imageTransitions.push_back({
            .image = mainImage,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = entityImage,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = depthImage,
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = mainImage->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = _graphicsState.renderArea.value(),
            .colorAttachments = _colorAttachments,
            .layerCount = 1
        };
    }

    void SelectionOutlinePass::PushConstants(const RenderContext& context)
    {
        auto scene = context.scene;
        auto settings = scene->GetSettings();

        Vk::PushConstant<SelectionOutlinePC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex, true);
        pc->outlinePrimaryColor = settings->outlinePrimaryColor;
        pc->outlineSecondaryColor = settings->outlineSecondaryColor;
        pc->outlineThickness = settings->outlineThickness;
        pc->enableSelectedOutline = settings->enableSelectedOutline ? 1 : 0;
        pc->enableSelectedHierarchyOutline = settings->enableSelectedHierarchyOutline ? 1 : 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SelectionOutlinePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();
 
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto entityTexture = rtGroup->GetImage(RenderTargetNames::EntityIndex);
        auto depthTexture = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto nearestSampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            entityTexture->GetView(Vk::ImageViewNames::Default),
            nearestSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            1,
            depthTexture->GetView(RenderTargetViewNames::DepthTransparentMin),
            nearestSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void SelectionOutlinePass::Draw(const RenderContext& context)
    {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}