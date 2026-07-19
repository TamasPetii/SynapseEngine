#include "MeshletOpaqueAlphaTestedForwardPass.h"
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
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/TraditionalMeshletPassPC.glsl"

    bool MeshletOpaqueAlphaTestedForwardPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->lighting.pipelineType == PipelineType::ForwardPlus
            && !context.scene->GetSettings()->debug.enableDebugVisibility;;
    }

    MeshletOpaqueAlphaTestedForwardPass::MeshletOpaqueAlphaTestedForwardPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "MeshletOpaqueAlphaTestedForward1Sided";
        }
        else {
            _passName = "MeshletOpaqueAlphaTestedForward2Sided";
        }
    }

    void MeshletOpaqueAlphaTestedForwardPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.defines = { ShaderDefines::EnableAlphaTest };
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return imageManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };

        _shaderProgram = shaderManager->CreateProgram("MeshletOpaqueAlphaTestedForwardProgram", {
            ShaderNames::MeshletTask,
            ShaderNames::MeshletMesh,
            ShaderNames::OpaqueForwardFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::AlphaTestedOpaque2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

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
                .compareOp = VK_COMPARE_OP_EQUAL
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

    void MeshletOpaqueAlphaTestedForwardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
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

    void MeshletOpaqueAlphaTestedForwardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<TraditionalMeshletPassPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.activeTraditionalCount + drawData->Models.meshletCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc->disableConeCulling = _renderType == MaterialRenderType::Opaque2Sided ? 1 : 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void MeshletOpaqueAlphaTestedForwardPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto drawData = context.scene->GetSceneDrawData();

        //Using prevous frame's depth pyramid!
        uint fIdx = context.frameIndex;
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto prevRtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, fIdx);

        auto depthPyramid = prevRtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        auto ssaoTexture = rtGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoSampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

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
            1,
            ssaoTexture->GetView(Vk::ImageViewNames::Default),
            ssaoSampler->Handle(),
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

        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void MeshletOpaqueAlphaTestedForwardPass::Draw(const RenderContext& context)
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