#include "MeshletRenderPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/System/RenderSystem.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/CameraComponent.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Animation/AnimationManager.h"

#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Syn {
    struct MeshletPushConstants {
        uint64_t modelAddressBuffer;

        uint64_t animationAddressBuffer;
        uint64_t animationBufferAddr;
        uint64_t animationSparseMapBufferAddr;

        uint64_t globalDrawCountBuffers;
        uint64_t globalInstanceBuffers;
        uint64_t globalIndirectCommandBuffers;
        uint64_t globalIndirectCommandDescriptorBuffers;
        uint64_t globalModelAllocationBuffers;
        uint64_t globalMeshAllocationBuffers;

        uint64_t cameraBufferAddr;
        uint64_t cameraSparseMapBufferAddr;
        uint64_t transformBufferAddr;
        uint64_t transformSparseMapBufferAddr;

        uint64_t modelBufferAddr;
        uint64_t modelSparseMapBufferAddr;
        uint64_t materialLookupBuffer;
        uint64_t materialBuffer;

        uint64_t debugInstanceBufferAddr;
        uint64_t debugAabbIndirectAddr;
        uint64_t debugSphereIndirectAddr;

        uint64_t prevCameraBufferAddr;
        uint64_t prevCameraSparseMapBufferAddr;

        uint32_t activeCameraEntity;
        uint32_t meshletOffsetStart;
        uint32_t visualizeMeshlet;

        float screenWidth;
        float screenHeight;
    };

    void MeshletRenderPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        auto imageManager = ServiceLocator::GetImageManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true; 
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return imageManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("MeshletProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::MeshletMesh,
            ShaderNames::MeshletFrag
            }, config);

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

    void MeshletRenderPass::PrepareFrame(const RenderContext& context) {
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

    void MeshletRenderPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto drawData = scene->GetSceneDrawData();
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;
        uint32_t prevFIdx = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        MeshletPushConstants pc{};
        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        pc.globalDrawCountBuffers = drawData->globalDrawCountBuffers[fIdx]->GetDeviceAddress();
        pc.globalInstanceBuffers = drawData->globalInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandBuffers = drawData->globalIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandDescriptorBuffers = drawData->globalIndirectCommandDescriptorBuffers[fIdx]->GetDeviceAddress();
        pc.globalModelAllocationBuffers = drawData->globalModelAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.globalMeshAllocationBuffers = drawData->globalMeshAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.transformBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.cameraBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.modelBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);
        pc.materialLookupBuffer = drawData->globalMaterialIndexBuffers[fIdx]->GetDeviceAddress();
        pc.materialBuffer = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        pc.debugInstanceBufferAddr = drawData->debugInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.debugAabbIndirectAddr = drawData->debugAabbIndirectBuffers[fIdx]->GetDeviceAddress();
        pc.debugSphereIndirectAddr = drawData->debugSphereIndirectBuffers[fIdx]->GetDeviceAddress();
        
        pc.prevCameraBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraData, prevFIdx);
        pc.prevCameraSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraSparseMap, prevFIdx);

        //pc.activeCameraEntity = scene->GetDebugCameraEntity();
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.meshletOffsetStart = SceneDrawData::MESHLET_OFFSET_START;
        pc.visualizeMeshlet = 0;
        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(MeshletPushConstants),
            &pc
        );
    }

    void MeshletRenderPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();

        //This deletes all pushdescriptors, need to call this first!
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void MeshletRenderPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto indirectBuffer = drawData->globalIndirectCommandBuffers[context.frameIndex]->Handle();
        auto countBuffer = drawData->globalDrawCountBuffers[context.frameIndex]->Handle();

        VkDeviceSize indirectOffset = SceneDrawData::MESHLET_OFFSET_START * sizeof(VkDrawIndirectCommand);
        VkDeviceSize countOffset = sizeof(uint32_t);

        vkCmdDrawMeshTasksIndirectCountEXT(
            context.cmd,
            indirectBuffer,
            indirectOffset,
            countBuffer,
            countOffset,
            SceneDrawData::MAX_INDIRECT_COMMANDS - SceneDrawData::MESHLET_OFFSET_START,
            sizeof(VkDrawMeshTasksIndirectCommandEXT)
        );
    }
}