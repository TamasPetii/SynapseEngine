#include "TraditionalRenderPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/Manager/ModelManager.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <windows.h>

namespace Syn {

    struct TraditionalPushConstants {
        VkDeviceAddress modelAddressBuffer;

        VkDeviceAddress globalDrawCountBuffers;
        VkDeviceAddress globalInstanceBuffers;
        VkDeviceAddress globalIndirectCommandBuffers;
        VkDeviceAddress globalIndirectCommandDescriptorBuffers;
        VkDeviceAddress globalModelAllocationBuffers;
        VkDeviceAddress globalMeshAllocationBuffers;

        VkDeviceAddress cameraBufferAddr;
        VkDeviceAddress cameraSparseMapBufferAddr;
        VkDeviceAddress transformBufferAddr;
        VkDeviceAddress transformSparseMapBufferAddr;
        VkDeviceAddress modelBufferAddr;
        VkDeviceAddress modelSparseMapBufferAddr;

        uint32_t activeCameraEntity;
        uint32_t meshletOffsetStart;

        uint32_t padding0;
        uint32_t padding1;
    };

    void TraditionalRenderPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        _shaderProgram = shaderManager->CreateProgram("TraditionalProgram", {
            ShaderNames::TraditionalVert,
            ShaderNames::TraditionalFrag
            });

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
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

    void TraditionalRenderPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        std::vector<std::string> targets = {
            RenderTargetNames::Main
        };

        for (const auto& name : targets) 
        {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                    .imageView = group->GetImage(name)->GetView(Vk::ImageViewNames::Default),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                }));
        }

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

    void TraditionalRenderPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();
        auto modelManager = ServiceLocator::GetModelManager();
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();

        TraditionalPushConstants pc{};

        // 1. Model Manager globális címei
        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();

        // 2. Jelenet culling és draw bufferei az aktuális frame-hez
        uint32_t fIdx = context.frameIndex;
        pc.globalDrawCountBuffers = drawData->globalDrawCountBuffers[fIdx]->GetDeviceAddress();
        pc.globalInstanceBuffers = drawData->globalInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandBuffers = drawData->globalIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandDescriptorBuffers = drawData->globalIndirectCommandDescriptorBuffers[fIdx]->GetDeviceAddress();
        pc.globalModelAllocationBuffers = drawData->globalModelAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.globalMeshAllocationBuffers = drawData->globalMeshAllocationBuffers[fIdx]->GetDeviceAddress();

        // 3. ECS Bufferek bekötése
        pc.transformBufferAddr = componentBufferManager->GetComponentBuffer(BufferNames::TransformData, fIdx).buffer->GetDeviceAddress();
        pc.transformSparseMapBufferAddr = componentBufferManager->GetComponentBuffer(BufferNames::TransformSparseMap, fIdx).buffer->GetDeviceAddress();
        pc.cameraBufferAddr = componentBufferManager->GetComponentBuffer(BufferNames::CameraData, fIdx).buffer->GetDeviceAddress();
        pc.cameraSparseMapBufferAddr = componentBufferManager->GetComponentBuffer(BufferNames::CameraSparseMap, fIdx).buffer->GetDeviceAddress();

        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.meshletOffsetStart = SceneDrawData::MESHLET_OFFSET_START;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(TraditionalPushConstants),
            &pc
        );
    }

    void TraditionalRenderPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->globalIndirectCommandBuffers[context.frameIndex]->Handle();
        auto countBuffer = drawData->globalDrawCountBuffers[context.frameIndex]->Handle();

        vkCmdDrawIndirectCount(
            context.cmd,
            indirectBuffer,
            0,
            countBuffer,
            0,
            SceneDrawData::SceneDrawData::MESHLET_OFFSET_START,
            sizeof(VkDrawIndirectCommand)
        );
    }
}