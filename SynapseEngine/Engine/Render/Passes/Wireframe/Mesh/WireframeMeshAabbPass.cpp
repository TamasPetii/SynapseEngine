#include "WireframeMeshAabbPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Animation/AnimationManager.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeMeshPC.glsl"

    bool WireframeMeshAabbPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableWireframeMeshAabb;
    }

    void WireframeMeshAabbPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("WireframeProgram", {
            ShaderNames::WireframeMeshVert,
            ShaderNames::WireframeFrag
            });

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
            .blendStates = {
                {
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
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

    void WireframeMeshAabbPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

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

    void WireframeMeshAabbPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        if (drawData->Models.activeDescriptorCount == 0) return;

        auto modelManager = ServiceLocator::GetModelManager();
        auto compManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        uint32_t fIdx = context.frameIndex;

        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
		auto isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        WireframeMeshPC pc{};
		pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, isGpu);
		pc.vertexPositionBufferAddr = cube->hardwareBuffers.vertexPositions->GetDeviceAddress();
		pc.indexBufferAddr = cube->hardwareBuffers.indices->GetDeviceAddress();
        pc.shapeType = WIREFRAME_MESH_SHAPE_TYPE_CUBE;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(WireframeMeshPC), &pc);
    }

    void WireframeMeshAabbPass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
		auto isGpu = scene->GetSettings()->enableGeometryGpuCulling;
		auto fIdx = context.frameIndex;

        if (totalCommands == 0) return;

        auto indirectBuffer = drawData->Debug.modelAabbIndirectBuffer.GetHandle(fIdx, isGpu);

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            totalCommands,
            sizeof(VkDrawIndirectCommand)
        );
    }
}