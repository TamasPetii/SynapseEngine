#include "PointLightBillboardPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Utils/PathUtils.h"

namespace Syn {
    #include "Engine/Shaders/Includes/PushConstants/BillboardPC.glsl"

    bool PointLightBillboardPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();

        if (!pool || pool->Size() == 0)
            return false;

        return context.scene->GetSettings()->enableBillboardPointLights;
    }

    void PointLightBillboardPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram(
            "PointLightBillboardProgram",
            {
                ShaderNames::BillboardVert,
                ShaderNames::BillboardFrag
            },
            config
        );

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .polygonMode = VK_POLYGON_MODE_FILL
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
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
                },
                {
                    .enable = VK_FALSE
                }
            },
            .colorAttachmentCount = 2,
        };

        _iconTexture = ServiceLocator::GetImageManager()->LoadImageSync(PathUtils::GetAbsolutePathString("Assets/PointLightIcon.png"));
    }

    void PointLightBillboardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };

        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::EntityIndex)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::TransparentDepth)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = context.scene->GetSettings()->enableGeometryGpuCulling;

        Vk::BufferCopyInfo copyRegion{};
        copyRegion.srcBuffer = drawData->PointLights.indirectBuffer.GetHandle(fIdx, isGpu);
        copyRegion.dstBuffer = drawData->PointLights.billboardSingleCmdBuffer.GetHandle(fIdx, isGpu);
        copyRegion.srcOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyRegion.dstOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyRegion.size = sizeof(uint32_t);

        Vk::BufferUtils::CopyBuffer(context.cmd, copyRegion);

        Vk::BufferBarrierInfo memBarrier{};
        memBarrier.buffer = drawData->PointLights.billboardSingleCmdBuffer.GetHandle(fIdx, isGpu);
        memBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        memBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        memBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        memBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

        Vk::BufferUtils::InsertBarrier(context.cmd, memBarrier);
    }

    void PointLightBillboardPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto texture = imageManager->GetResource(_iconTexture);
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge)->Handle();

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            texture->image->GetView(Vk::ImageViewNames::Default),
            sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void PointLightBillboardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<BillboardPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc->visibleEntitiesAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        pc->baseScale = 1.0f;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightBillboardPass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        vkCmdDrawIndirect(
            context.cmd,
            drawData->PointLights.billboardSingleCmdBuffer.GetHandle(fIdx, isGpu),
            0,
            1,
            sizeof(VkDrawIndirectCommand)
        );
    }
}