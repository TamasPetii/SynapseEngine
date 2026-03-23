#include "WireframeSpherePass.h"
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


namespace Syn {

    struct WireframePushConstants {
        VkDeviceAddress animationAddressBuffer;
        VkDeviceAddress animationBufferAddr;
        VkDeviceAddress animationSparseMapBufferAddr;
        VkDeviceAddress modelAddressBuffer;
        VkDeviceAddress globalInstanceBuffers;
        VkDeviceAddress globalIndirectCommandDescriptorBuffers;
        VkDeviceAddress cameraBufferAddr;
        VkDeviceAddress cameraSparseMapBufferAddr;
        VkDeviceAddress transformBufferAddr;
        VkDeviceAddress transformSparseMapBufferAddr;
        VkDeviceAddress indexBufferAddr;
        VkDeviceAddress vertexBufferAddr;
        uint32_t activeCameraEntity;
        uint32_t isSphere;
        uint32_t drawIdOffset;
        alignas(16) glm::vec4 debugColor;
    };

    void WireframeSpherePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("WireframeProgram", {
            ShaderNames::WireframeVert,
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

    void WireframeSpherePass::PrepareFrame(const RenderContext& context) {
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

    void WireframeSpherePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();
        if (drawData->activeDescriptorCount == 0) return;

        auto modelManager = ServiceLocator::GetModelManager();
        auto compManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();
        uint32_t fIdx = context.frameIndex;

        auto sphereMesh = modelManager->GetResource(MeshSourceNames::Sphere);
        if (!sphereMesh) return;

        WireframePushConstants pc{};
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.globalInstanceBuffers = drawData->globalInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandDescriptorBuffers = drawData->globalIndirectCommandDescriptorBuffers[fIdx]->GetDeviceAddress();
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);

        pc.indexBufferAddr = sphereMesh->hardwareBuffers.indices->GetDeviceAddress();
        pc.vertexBufferAddr = sphereMesh->hardwareBuffers.vertexPositions->GetDeviceAddress();

        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.isSphere = 1;
        pc.debugColor = glm::vec4(1, 0, 0, 1);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(WireframePushConstants), &pc);
    }

    void WireframeSpherePass::Draw(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        if (!scene || drawData->activeDescriptorCount == 0) return;

        auto indirectBuffer = drawData->sphereIndirectCommandBuffers[context.frameIndex]->Handle();
        auto countBuffer = drawData->globalDrawCountBuffers[context.frameIndex]->Handle();

        //Traditional meshek Sphere
        uint32_t offsetTraditional = 0;
        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            offsetof(WireframePushConstants, drawIdOffset),
            sizeof(uint32_t),
            &offsetTraditional
        );

        vkCmdDrawIndirectCount(
            context.cmd,
            indirectBuffer,
            0,
            countBuffer,
            0,
            SceneDrawData::MESHLET_OFFSET_START,
            sizeof(VkDrawIndirectCommand)
        );

        // Meshlet meshek Sphere
        uint32_t offsetMeshlet = SceneDrawData::MESHLET_OFFSET_START;
        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            offsetof(WireframePushConstants, drawIdOffset),
            sizeof(uint32_t),
            &offsetMeshlet
        );

        VkDeviceSize meshletIndirectOffset = SceneDrawData::MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        VkDeviceSize meshletCountOffset = sizeof(uint32_t);

        vkCmdDrawIndirectCount(
            context.cmd,
            indirectBuffer,
            meshletIndirectOffset,
            countBuffer,
            meshletCountOffset,
            SceneDrawData::MAX_INDIRECT_COMMANDS - SceneDrawData::MESHLET_OFFSET_START,
            sizeof(VkDrawIndirectCommand)
        );
    }
}