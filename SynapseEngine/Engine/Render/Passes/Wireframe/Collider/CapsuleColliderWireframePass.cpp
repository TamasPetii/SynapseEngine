#include "CapsuleColliderWireframePass.h"
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
#include "Engine/Component/Physics/CapsuleColliderComponent.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/WireframeDebugPC.glsl"

    bool CapsuleColliderWireframePass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableCapsuleColliderWireframe;
    }

    void CapsuleColliderWireframePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram(
            "DebugWireframeProgram",
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

    void CapsuleColliderWireframePass::PrepareFrame(const RenderContext& context) {
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

        auto registry = scene->GetRegistry();
        auto pool = registry->GetPool<CapsuleColliderComponent>();

        _activeColliderCount = pool ? static_cast<uint32_t>(pool->Size()) : 0;

        VkDrawIndirectCommand cmd = drawData->Debug.capsuleColliderCmdTemplate;
        cmd.instanceCount = _activeColliderCount;
        drawData->Debug.capsuleColliderIndirectBuffer.GetMapped(fIdx)->Write(&cmd, sizeof(VkDrawIndirectCommand), 0);
    }

    void CapsuleColliderWireframePass::PushConstants(const RenderContext& context) {
        if (_activeColliderCount == 0) return;

        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t fIdx = context.frameIndex;

        auto capsule = modelManager->GetResource(MeshSourceNames::Capsule);

        WireframeDebugPC pc{};
        pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc.vertexPositionBufferAddr = capsule->hardwareBuffers.vertexPositions->GetDeviceAddress();
        pc.indexBufferAddr = capsule->hardwareBuffers.indices->GetDeviceAddress();
        pc.shapeDrawType = WIREFRAME_DEBUG_SHAPE_TYPE_CAPSULE_COLLIDER;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(WireframeDebugPC),
            &pc
        );
    }

    void CapsuleColliderWireframePass::Draw(const RenderContext& context) {
        if (_activeColliderCount == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto indirectBuffer = drawData->Debug.capsuleColliderIndirectBuffer.GetHandle(fIdx, false);
        vkCmdDrawIndirect(context.cmd, indirectBuffer, 0, 1, sizeof(VkDrawIndirectCommand));
    }
}