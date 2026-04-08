#include "PerformanceMeshletOpaquePass.h"
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
#include <cassert>

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/MeshletPassPC.glsl"

    PerformanceMeshletOpaquePass::PerformanceMeshletOpaquePass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::Opaque1Sided || _renderType == MaterialRenderType::Opaque2Sided);
        _passName = (_renderType == MaterialRenderType::Opaque1Sided) ? "Perf_Meshlet_Opaque_1Sided" : "Perf_Meshlet_Opaque_2Sided";
    }

    void PerformanceMeshletOpaquePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        auto imageManager = ServiceLocator::GetImageManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("PerformanceMeshletOpaqueProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::PerformanceMeshletMesh,
            ShaderNames::PerformanceMeshletFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::Opaque2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = cullMode,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
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

    void PerformanceMeshletOpaquePass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        if (auto mainImg = group->GetImage(RenderTargetNames::Main)) {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = mainImg->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                }));
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::Depth)) {
            _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = depthImg->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                });
        }

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = _depthAttachment.has_value() ? &_depthAttachment.value() : nullptr,
            .layerCount = 1
        };
    }

    void PerformanceMeshletOpaquePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto drawData = scene->GetSceneDrawData();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;

        MeshletPassPC pc{};
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

        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.mainCameraEntity = scene->GetSceneCameraEntity();
        
        pc.baseDescriptorOffset = drawData->activeTraditionalCount + drawData->meshletCmdOffsets[_renderType];
        pc.disableConeCulling = (_renderType == MaterialRenderType::Opaque2Sided) ? 1 : 0;
        pc.materialRenderType = static_cast<uint32_t>(_renderType);
        pc.enableOcclusionCulling = (scene->GetSettings()->enableGpuCulling && scene->GetSettings()->enableOcclusionCulling && !scene->GetSettings()->useDebugCamera) ? 1 : 0;

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(MeshletPassPC),
            &pc
        );
    }

    void PerformanceMeshletOpaquePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();


        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);

        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void PerformanceMeshletOpaquePass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto indirectBuffer = drawData->globalIndirectCommandBuffers[context.frameIndex]->Handle();
        auto countBuffer = drawData->globalDrawCountBuffers[context.frameIndex]->Handle();

        uint32_t commandOffsetIdx = drawData->meshletCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->meshletCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize traditionalBytes = drawData->activeTraditionalCount * sizeof(VkDrawIndirectCommand);
            VkDeviceSize indirectOffset = traditionalBytes + (commandOffsetIdx * sizeof(VkDrawMeshTasksIndirectCommandEXT));
            VkDeviceSize countOffset = (MaterialRenderType::Count + _renderType) * sizeof(uint32_t);

            vkCmdDrawMeshTasksIndirectCountEXT(
                context.cmd,
                indirectBuffer,
                indirectOffset,
                countBuffer,
                countOffset,
                maxCommandCount,
                sizeof(VkDrawMeshTasksIndirectCommandEXT)
            );
        }
    }
}