#include "WireframeMeshSpherePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeMeshPC.glsl"

    bool WireframeMeshSpherePass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->debug.enableWireframeMeshSphere;
    }

    void WireframeMeshSpherePass::Initialize() {
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
            } ,
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void WireframeMeshSpherePass::PrepareFrame(const RenderContext& context) {
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

    void WireframeMeshSpherePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto drawData = scene->GetSceneDrawData();
        if (drawData->Models.activeDescriptorCount == 0) return;

        auto modelManager = ServiceLocator::GetModelManager();
        auto compManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        uint32_t fIdx = context.frameIndex;

        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);

        Vk::PushConstant<WireframeMeshPC> pc{};
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
		pc->vertexPositionBufferAddr = sphere->hardwareBuffers.vertexPositions->GetDeviceAddress();
		pc->indexBufferAddr = sphere->hardwareBuffers.indices->GetDeviceAddress();
        pc->shapeType = WIREFRAME_MESH_SHAPE_TYPE_SPHERE;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void WireframeMeshSpherePass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

        if (totalCommands == 0) return;

		auto fIdx = context.frameIndex;

        auto indirectBuffer = drawData->Debug.modelSphereIndirectBuffer.GetHandle(fIdx);

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            totalCommands,
            sizeof(VkDrawIndirectCommand)
        );
    }
}