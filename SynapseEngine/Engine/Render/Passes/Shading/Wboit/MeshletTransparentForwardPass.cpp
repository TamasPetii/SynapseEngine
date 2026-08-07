#include "MeshletTransparentForwardPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
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

#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Video/VideoManager.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/TraditionalMeshletPassPC.glsl"

    MeshletTransparentForwardPass::MeshletTransparentForwardPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        switch (_renderType) {
        case MaterialRenderType::Transparent1Sided:
            _passName = "MeshletTransparentForward1Sided";
            break;
        case MaterialRenderType::Transparent2Sided:
            _passName = "MeshletTransparentForward2Sided";
            break;
        case MaterialRenderType::AlphaTestedTransparent1Sided:
            _passName = "MeshletAlphaTestedTransparentForward1Sided";
            break;
        case MaterialRenderType::AlphaTestedTransparent2Sided:
            _passName = "MeshletAlphaTestedTransparentForward2Sided";
            break;
        default:
            assert(false && "Invalid RenderType for Transparent Pass!");
            break;
        }
    }

    bool MeshletTransparentForwardPass::ShouldExecute(const RenderContext& context) const
    {
        //Todo: Has transparent material?
        return !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void MeshletTransparentForwardPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto videoManager = ServiceLocator::Get<VideoManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [imageManager, videoManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return imageManager->GetBindlessLayout();
            }
            if (setIndex == 1) {
                return videoManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };

        _shaderProgramId = shaderManager->LoadProgramAsync("MeshletTransparentForwardProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::MeshletMesh,
            ShaderNames::TransparentForwardFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::Transparent2Sided || _renderType == MaterialRenderType::AlphaTestedTransparent2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

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

    void MeshletTransparentForwardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        uint32_t msaaSamples = context.scene->GetSettings()->lighting.msaaSamples;
        if (context.scene->GetSettings()->lighting.pipelineType == PipelineType::Deferred) {
            msaaSamples = 1;
        }
        _graphicsState.raster.samples = static_cast<VkSampleCountFlagBits>(msaaSamples);

        if (msaaSamples > 1) {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::TransparentAccumMSAA)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .resolveImageView = group->GetImage(RenderTargetNames::TransparentAccum)->GetView(Vk::ImageViewNames::Default),
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT
                }));

            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::TransparentRevealMSAA)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .resolveImageView = group->GetImage(RenderTargetNames::TransparentReveal)->GetView(Vk::ImageViewNames::Default),
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT
                }));

            _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::OpaqueDepthMSAA)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .resolveMode = VK_RESOLVE_MODE_NONE
                });

        }
        else {
            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::TransparentAccum)->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                }));

            _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
                .imageView = group->GetImage(RenderTargetNames::TransparentReveal)->GetView(Vk::ImageViewNames::Default),
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
        }

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void MeshletTransparentForwardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto materialManager = ServiceLocator::Get<MaterialManager>();

        auto drawData = scene->GetSceneDrawData();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto animationManager = ServiceLocator::Get<AnimationManager>();

        uint32_t fIdx = context.frameIndex;
        

        Vk::PushConstant<TraditionalMeshletPassPC> pc;
		pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.activeTraditionalCount + drawData->Models.meshletCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc->disableConeCulling = (_renderType == MaterialRenderType::Transparent2Sided) ? 1 : 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void MeshletTransparentForwardPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto drawData = context.scene->GetSceneDrawData();

        //Using prevous frame's depth pyramid!
        uint fIdx = context.frameIndex;
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        auto dirShadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto pointShadowAtlas = drawData->PointLightShadow.shadowAtlas[fIdx].get();
        auto spotShadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx].get();
        auto shadowSampler = imageManager->GetSampler(SamplerNames::ShadowSampler);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            2,
            dirShadowAtlas->GetView(Vk::ImageViewNames::Default),
            shadowSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            3,
            pointShadowAtlas->GetView(Vk::ImageViewNames::Default),
            shadowSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            4,
            spotShadowAtlas->GetView(Vk::ImageViewNames::Default),
            shadowSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);

        auto videoManager = ServiceLocator::Get<VideoManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;

        if (auto imgBuffer = imageManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, imgBuffer });
        }

        if (auto vidBuffer = videoManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 1, vidBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void MeshletTransparentForwardPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        

        auto indirectBuffer = drawData->Models.indirectBuffer.GetHandle(context.frameIndex);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex);

        uint32_t commandOffsetIdx = drawData->Models.meshletCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.meshletCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize traditionalBytes = drawData->Models.activeTraditionalCount * sizeof(VkDrawIndirectCommand);
            VkDeviceSize indirectOffset = traditionalBytes + (commandOffsetIdx * sizeof(VkDrawMeshTasksIndirectCommandEXT));
            VkDeviceSize countOffset = (MaterialRenderType::MaterialRenderTypeCount + _renderType) * sizeof(uint32_t);

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