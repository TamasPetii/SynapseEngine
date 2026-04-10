#include "PerformanceTraditionalOpaquePass.h"
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
#include "Engine/Image/ImageManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/AnimationManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/TraditionalPassPC.glsl"

    PerformanceTraditionalOpaquePass::PerformanceTraditionalOpaquePass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::Opaque1Sided || _renderType == MaterialRenderType::Opaque2Sided);
        _passName = (_renderType == MaterialRenderType::Opaque1Sided) ? "Perf_Traditional_Opaque_1Sided" : "Perf_Traditional_Opaque_2Sided";
    }

    void PerformanceTraditionalOpaquePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        auto imageManager = ServiceLocator::GetImageManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("PerformanceTraditionalOpaqueProgram", {
            ShaderNames::PerformanceTraditionalVert,
            ShaderNames::PerformanceTraditionalFrag
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

    void PerformanceTraditionalOpaquePass::PrepareFrame(const RenderContext& context) {
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

    void PerformanceTraditionalOpaquePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;

        TraditionalPassPC pc{};
        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.animationBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        pc.globalDrawCountBuffers = drawData->mappedDrawCountBuffers[fIdx]->GetDeviceAddress();
        pc.globalInstanceBuffers = context.scene->GetSettings()->enableGpuCulling ? drawData->gpuInstanceBuffers[fIdx]->GetDeviceAddress() : drawData->mappedInstanceBuffers[fIdx]->GetDeviceAddress();
        pc.globalIndirectCommandBuffers = context.scene->GetSettings()->enableGpuCulling ? drawData->gpuIndirectCommandBuffers[fIdx]->GetDeviceAddress() : drawData->mappedIndirectCommandBuffers[fIdx]->GetDeviceAddress();;
        pc.globalIndirectCommandDescriptorBuffers = drawData->gpuIndirectCommandDescriptorBuffers[fIdx]->GetDeviceAddress();
        pc.globalModelAllocationBuffers = drawData->gpuModelAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.globalMeshAllocationBuffers = drawData->gpuMeshAllocationBuffers[fIdx]->GetDeviceAddress();
        pc.transformBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.cameraBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.modelBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);
        pc.materialLookupBuffer = drawData->gpuMaterialIndexBuffers[fIdx]->GetDeviceAddress();
        pc.materialBuffer = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.baseDescriptorOffset = drawData->traditionalCmdOffsets[_renderType];
        pc.materialRenderType = static_cast<uint32_t>(_renderType);

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(TraditionalPassPC),
            &pc
        );
    }

    void PerformanceTraditionalOpaquePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void PerformanceTraditionalOpaquePass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        bool useGpuCulling = scene->GetSettings()->enableGpuCulling;

        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = useGpuCulling
            ? drawData->gpuIndirectCommandBuffers[context.frameIndex]->Handle()
            : drawData->mappedIndirectCommandBuffers[context.frameIndex]->Handle();

        auto countBuffer = drawData->mappedDrawCountBuffers[context.frameIndex]->Handle();

        uint32_t commandOffset = drawData->traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->traditionalCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize countBufferOffset = _renderType * sizeof(uint32_t);

            vkCmdDrawIndirectCount(
                context.cmd,
                indirectBuffer,
                commandOffset * sizeof(VkDrawIndirectCommand),
                countBuffer,
                countBufferOffset,
                maxCommandCount,
                sizeof(VkDrawIndirectCommand)
            );
        }
    }
}