#include "SpotLightShadowMeshletOpaquePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Scene/DrawData/SpotLightShadowDrawGroup.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/SpotLightShadowTraditionalMeshletPassPC.glsl"

    bool SpotLightShadowMeshletOpaquePass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    SpotLightShadowMeshletOpaquePass::SpotLightShadowMeshletOpaquePass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::Opaque1Sided || _renderType == MaterialRenderType::Opaque2Sided);

        if (_renderType == MaterialRenderType::Opaque1Sided) {
            _passName = "SpotLightShadowMeshletOpaquePass1Sided";
        }
        else {
            _passName = "SpotLightShadowMeshletOpaquePass2Sided";
        }
    }

    void SpotLightShadowMeshletOpaquePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("SpotLightShadowMeshletProgram", {
            ShaderNames::SpotLightShadowMeshletTask,
            ShaderNames::SpotLightShadowMeshletMesh,
            ShaderNames::SpotLightShadowFrag
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
            .blendStates = {},
            .colorAttachmentCount = 0,
            .renderArea = std::nullopt
        };
    }

    void SpotLightShadowMeshletOpaquePass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto& shadowGroup = drawData->SpotLightShadow;
        auto fIdx = context.frameIndex;

        VkExtent2D extent = { SPOT_SHADOW_ATLAS_SIZE, SPOT_SHADOW_ATLAS_SIZE };
        _graphicsState.renderArea = extent;

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = shadowGroup.shadowAtlas[fIdx]->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = {},
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void SpotLightShadowMeshletOpaquePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<SpotLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.activeTraditionalCount + drawData->Models.meshletCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowMeshletOpaquePass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::GetImageManager();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        auto depthPyramid = context.scene->GetSceneDrawData()->SpotLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void SpotLightShadowMeshletOpaquePass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        

        auto indirectBuffer = drawData->SpotLightShadow.indirectBuffer.GetHandle(context.frameIndex);
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