#include "TraditionalOpaquePass.h"
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

    TraditionalOpaquePass::TraditionalOpaquePass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::Opaque1Sided || _renderType == MaterialRenderType::Opaque2Sided);

        if (_renderType == MaterialRenderType::Opaque1Sided) {
            _passName = "Traditional_Opaque_1Sided";
        }
        else {
            _passName = "Traditional_Opaque_2Sided";
        }
    }

    void TraditionalOpaquePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        auto imageManager = ServiceLocator::GetImageManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) return imageManager->GetBindlessLayout();
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("TraditionalOpaqueProgram", {
            ShaderNames::TraditionalVert,
            ShaderNames::TraditionalFrag
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
            .colorAttachmentCount = 7,
            .renderArea = std::nullopt
        };
    }

    void TraditionalOpaquePass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        std::vector<std::string> targets = {
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo,
            RenderTargetNames::EntityIndex,
            RenderTargetNames::DebugTopologyPipeline,
            RenderTargetNames::DebugMeshletLod,
            RenderTargetNames::DebugMaterialUv
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

    void TraditionalOpaquePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        auto drawData = scene->GetSceneDrawData();
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        TraditionalPassPC pc{};
        pc.modelAddressBuffer = modelManager->GetModelAddressBuffer()->GetDeviceAddress();
        pc.animationAddressBuffer = animationManager->GetAnimationAddressBuffer()->GetDeviceAddress();
        pc.materialBuffer = materialManager->GetMaterialBuffer()->GetDeviceAddress();

        pc.animationBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationData, fIdx);
        pc.animationSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::AnimationSparseMap, fIdx);
        pc.transformBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.cameraBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.modelBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelData, fIdx);
        pc.modelSparseMapBufferAddr = componentBufferManager->GetBufferAddr(BufferNames::ModelSparseMap, fIdx);

        pc.globalDrawCountBuffers = drawData->Models.drawCountBuffer.GetAddress(fIdx, isGpu);
        pc.globalInstanceBuffers = drawData->Models.instanceBuffer.GetAddress(fIdx, isGpu);
        pc.globalIndirectCommandBuffers = drawData->Models.indirectBuffer.GetAddress(fIdx, isGpu);
        pc.globalIndirectCommandDescriptorBuffers = drawData->Models.descriptorBuffer.GetAddress(fIdx, isGpu);
        pc.globalModelAllocationBuffers = drawData->Models.modelAllocBuffer.GetAddress(fIdx, isGpu);
        pc.globalMeshAllocationBuffers = drawData->Models.meshAllocBuffer.GetAddress(fIdx, isGpu);
        pc.materialLookupBuffer = drawData->Models.materialIndexBuffer.GetAddress(fIdx, isGpu);
        
        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
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

    void TraditionalOpaquePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void TraditionalOpaquePass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        auto indirectBuffer = drawData->Models.indirectBuffer.GetHandle(context.frameIndex, isGpu);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex, isGpu);

        uint32_t commandOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.traditionalCmdCounts[_renderType];

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