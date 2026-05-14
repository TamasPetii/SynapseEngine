#include "MeshletTransparentForwardPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Mesh/ModelManager.h"
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

    #include "Engine/Shaders/Includes/PushConstants/TraditionalMeshletPassPC.glsl"

    MeshletTransparentForwardPass::MeshletTransparentForwardPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        _passName = (_renderType == MaterialRenderType::Transparent1Sided) ? "Meshlet_Transparent_Forward_1Sided" : "Meshlet_Transparent_Forward_2Sided";
    }

    void MeshletTransparentForwardPass::Initialize() {
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

        _shaderProgram = shaderManager->CreateProgram("MeshletTransparentForwardProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::MeshletMesh,
            ShaderNames::TransparentForwardFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::Transparent2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

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
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blendStates = {
                // 0. TransparentAccum
                {
                    .enable = true,
                    .srcColorFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                },
                // 1. TransparentReveal
                {
                    .enable = true,
                    .srcColorFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 2,
            .renderArea = std::nullopt
        };
    }

    bool MeshletTransparentForwardPass::ShouldExecute(const RenderContext& context) const
    {
        return !context.scene->GetSettings()->enableDebugVisibility;
    }

    void MeshletTransparentForwardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        std::vector<std::string> targets = {
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal
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

    void MeshletTransparentForwardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto modelManager = ServiceLocator::GetModelManager();
        auto materialManager = ServiceLocator::GetMaterialManager();

        auto drawData = scene->GetSceneDrawData();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto animationManager = ServiceLocator::GetAnimationManager();

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        TraditionalMeshletPassPC pc{};
		pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc.baseDescriptorOffset = drawData->Models.activeTraditionalCount + drawData->Models.meshletCmdOffsets[_renderType];
        pc.materialRenderType = static_cast<uint32_t>(_renderType);
        pc.disableConeCulling = (_renderType == MaterialRenderType::Transparent2Sided) ? 1 : 0;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(TraditionalMeshletPassPC),
            &pc
        );
    }

    void MeshletTransparentForwardPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();

        //Using prevous frame's depth pyramid!
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);

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

    void MeshletTransparentForwardPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        auto indirectBuffer = drawData->Models.indirectBuffer.GetHandle(context.frameIndex, isGpu);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex, isGpu);

        uint32_t commandOffsetIdx = drawData->Models.meshletCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.meshletCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize traditionalBytes = drawData->Models.activeTraditionalCount * sizeof(VkDrawIndirectCommand);
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