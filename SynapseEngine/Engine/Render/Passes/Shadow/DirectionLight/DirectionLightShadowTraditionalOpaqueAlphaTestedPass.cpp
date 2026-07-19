#include "DirectionLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowTraditionalMeshletPassPC.glsl"

    bool DirectionLightShadowTraditionalOpaqueAlphaTestedPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    DirectionLightShadowTraditionalOpaqueAlphaTestedPass::DirectionLightShadowTraditionalOpaqueAlphaTestedPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "DirectionLightShadowTraditionalOpaqueAlphaTestedPass1Sided";
        }
        else {
            _passName = "DirectionLightShadowTraditionalOpaqueAlphaTestedPass2Sided";
        }
    }

    void DirectionLightShadowTraditionalOpaqueAlphaTestedPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;
        config.defines = { "ENABLE_ALPHA_TEST" };

        _shaderProgram = shaderManager->CreateProgram("DirectionLightShadowOpaqueAlphaTestedProgram", {
            ShaderNames::DirectionLightShadowTraditionalVert,
            ShaderNames::DirectionLightShadowFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::AlphaTestedOpaque2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = cullMode,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
                /* .depthBiasEnable = VK_TRUE,*/
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

    void DirectionLightShadowTraditionalOpaqueAlphaTestedPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto& shadowGroup = drawData->DirectionLightShadow;
        auto fIdx = context.frameIndex;

        VkExtent2D extent = { SHADOW_ATLAS_SIZE, SHADOW_ATLAS_SIZE };
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

    void DirectionLightShadowTraditionalOpaqueAlphaTestedPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        uint32_t fIdx = context.frameIndex;

        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<DirectionLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowTraditionalOpaqueAlphaTestedPass::BindDescriptors(const RenderContext& context)
    {

    }

    void DirectionLightShadowTraditionalOpaqueAlphaTestedPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();


        auto indirectBuffer = drawData->DirectionLightShadow.indirectBuffer.GetHandle(context.frameIndex);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex);

        uint32_t commandOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.traditionalCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize countBufferOffset = _renderType * sizeof(uint32_t);
            VkDeviceSize indirectOffset = commandOffset * sizeof(VkDrawIndirectCommand);

            vkCmdDrawIndirectCount(
                context.cmd,
                indirectBuffer,
                indirectOffset,
                countBuffer,
                countBufferOffset,
                maxCommandCount,
                sizeof(VkDrawIndirectCommand)
            );
        }
    }
}