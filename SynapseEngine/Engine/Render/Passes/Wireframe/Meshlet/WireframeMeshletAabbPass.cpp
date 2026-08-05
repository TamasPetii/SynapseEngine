#include "WireframeMeshletAabbPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/WireframeMeshletPC.glsl"

    bool WireframeMeshletAabbPass::ShouldExecute(const RenderContext& context) const {

        return context.scene->GetSettings()->debug.enableWireframeMeshletAabb;
    }

    void WireframeMeshletAabbPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("WireframeMeshletAabbProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::WireframeMeshletMesh,
            ShaderNames::WireframeFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_LINE,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blendStates = {{
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD
            }},
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void WireframeMeshletAabbPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

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
    }

    void WireframeMeshletAabbPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        if (drawData->Models.activeMeshletCount == 0) return;

        auto modelManager = ServiceLocator::Get<ModelManager>();
        uint32_t fIdx = context.frameIndex;
        

        auto shape = modelManager->GetResource(MeshSourceNames::Cube);

        Vk::PushConstant<WireframeMeshletPC> pc{};
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->vertexPositionBufferAddr = shape->hardwareBuffers.vertexPositions->GetDeviceAddress();
        pc->indexBufferAddr = shape->hardwareBuffers.indices->GetDeviceAddress();
        pc->baseDescriptorOffset = drawData->Models.activeTraditionalCount;
        pc->vertexCount = shape->cpuData.globalVertexCount;
        pc->indexCount = shape->cpuData.baseDrawCommands[0].traditionalCmd.vertexCount;
        pc->shapeType = WIREFRAME_MESHLET_SHAPE_TYPE_CUBE;
        pc->materialRenderType = 0;
        pc->disableConeCulling = 1;
		pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void WireframeMeshletAabbPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto prevRtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto depthPyramid = prevRtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void WireframeMeshletAabbPass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        if (drawData->Models.activeMeshletCount == 0) return;

        
        auto indirectBuffer = drawData->Models.indirectBuffer.GetHandle(context.frameIndex);

        VkDeviceSize traditionalBytes = drawData->Models.activeTraditionalCount * sizeof(VkDrawIndirectCommand);

        vkCmdDrawMeshTasksIndirectEXT(
            context.cmd,
            indirectBuffer,
            traditionalBytes,
            drawData->Models.activeMeshletCount,
            sizeof(VkDrawMeshTasksIndirectCommandEXT)
        );
    }
}