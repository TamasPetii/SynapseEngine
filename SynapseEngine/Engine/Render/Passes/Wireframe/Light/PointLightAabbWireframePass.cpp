#include "PointLightAabbWireframePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeDebugPC.glsl"

    bool PointLightAabbWireframePass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enablePointLightAabbWireframe;
    }

    void PointLightAabbWireframePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram(
            "WireframeLightProgram",
            {
                ShaderNames::WireframeDebugVert,
                ShaderNames::WireframeFrag
            },
            config
        );

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .polygonMode = VK_POLYGON_MODE_LINE,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .colorAttachmentCount = 1
        };
    }

    void PointLightAabbWireframePass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };

        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(
            Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                })
        );

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::OpaqueDepth)->GetView(Vk::ImageViewNames::Default),
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

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
		auto isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        Vk::BufferCopyInfo copyRegion{};
        copyRegion.srcBuffer = drawData->PointLights.indirectBuffer.GetHandle(fIdx, isGpu);
        copyRegion.dstBuffer = drawData->PointLights.aabbSingleCmdBuffer.GetHandle(fIdx, isGpu);
        copyRegion.srcOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyRegion.dstOffset = offsetof(VkDrawIndirectCommand, instanceCount);
        copyRegion.size = sizeof(uint32_t);

        Vk::BufferUtils::CopyBuffer(context.cmd, copyRegion);

        Vk::BufferBarrierInfo memBarrier{};
        memBarrier.buffer = drawData->PointLights.aabbSingleCmdBuffer.GetHandle(fIdx, isGpu);
        memBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        memBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        memBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        memBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

        Vk::BufferUtils::InsertBarrier(context.cmd, memBarrier);
    }

    void PointLightAabbWireframePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t fIdx = context.frameIndex;

        auto cube = modelManager->GetResource(MeshSourceNames::Cube);

        WireframeDebugPC pc{};
		pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc.vertexPositionBufferAddr = cube->hardwareBuffers.vertexPositions->GetDeviceAddress();
		pc.indexBufferAddr = cube->hardwareBuffers.indices->GetDeviceAddress();
		pc.lightDrawType = 1;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(WireframeDebugPC),
            &pc
        );
    }

    void PointLightAabbWireframePass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        auto isGpu = scene->GetSettings()->enableGeometryGpuCulling;

		auto indirectBuffer = drawData->PointLights.aabbSingleCmdBuffer.GetHandle(fIdx, isGpu);

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            1,
            sizeof(VkDrawIndirectCommand)
        );
    }
}