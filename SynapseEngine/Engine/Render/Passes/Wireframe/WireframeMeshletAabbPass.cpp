#include "WireframeMeshletAabbPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneDrawData.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Animation/AnimationManager.h"

namespace Syn
{
    #include "Engine/Shaders/Includes/PushConstants/WireframePC.glsl"

    bool WireframeMeshletAabbPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableWireframeMeshletAabb;
    }

    void WireframeMeshletAabbPass::Initialize()
    {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("WireframeMeshletProgram", {
            ShaderNames::WireframeMeshletVert,
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
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
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

    void WireframeMeshletAabbPass::PrepareFrame(const RenderContext& context)
    {
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
            .imageView = group->GetImage(RenderTargetNames::Depth)->GetView(Vk::ImageViewNames::Default),
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

    void WireframeMeshletAabbPass::PushConstants(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        if (drawData->activeDescriptorCount == 0 || !drawData->debugInstanceBuffers[fIdx] || !drawData->debugAabbIndirectBuffers[fIdx]) return;

        auto modelManager = ServiceLocator::GetModelManager();
        auto compManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        auto cubeMesh = modelManager->GetResource(MeshSourceNames::Cube);
        if (!cubeMesh) return;

        WireframePC pc{};
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);

        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.globalInstanceBuffers = context.scene->GetSettings()->enableGpuCulling ? drawData->gpuInstanceBuffers[fIdx]->GetDeviceAddress() : drawData->mappedInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandDescriptorBuffers = drawData->gpuIndirectCommandDescriptorBuffers[fIdx]->GetDeviceAddress();

        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);

        pc.modelBufferAddr = compManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);

        pc.indexBufferAddr = cubeMesh->hardwareBuffers.indices->GetDeviceAddress();
        pc.vertexBufferAddr = cubeMesh->hardwareBuffers.vertexPositions->GetDeviceAddress();
        pc.debugInstanceBufferAddr = drawData->debugInstanceBuffers[fIdx]->GetDeviceAddress();

        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.isSphere = 0;
        pc.drawIdOffset = 0;
        pc.debugColor = glm::vec4(0.0, 1.0, 0.0, 1.0);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(WireframePC), &pc);
    }

    void WireframeMeshletAabbPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        if (drawData->activeDescriptorCount == 0 || !drawData->debugAabbIndirectBuffers[context.frameIndex]) return;

        auto indirectBuffer = drawData->debugAabbIndirectBuffers[context.frameIndex]->Handle();

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            drawData->totalMaxMeshletInstances,
            sizeof(VkDrawIndirectCommand)
        );
    }
}