#include "PointLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Scene/DrawData/PointLightShadowDrawGroup.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/PointLightShadowTraditionalMeshletPassPC.glsl"

    bool PointLightShadowTraditionalOpaqueAlphaTestedPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    PointLightShadowTraditionalOpaqueAlphaTestedPass::PointLightShadowTraditionalOpaqueAlphaTestedPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "PointLightShadowTraditionalOpaqueAlphaTestedPass1Sided";
        }
        else {
            _passName = "PointLightShadowTraditionalOpaqueAlphaTestedPass2Sided";
        }
    }

    void PointLightShadowTraditionalOpaqueAlphaTestedPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();

        Vk::ShaderProgramConfig config;
        config.layoutOverride = [imageManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return imageManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };
        config.defines = { ShaderDefines::EnableAlphaTest };

        _shaderProgram = shaderManager->CreateProgram("PointLightShadowTraditionalAlphaTestedProgram", {
            ShaderNames::PointLightShadowTraditionalVert,
            ShaderNames::PointLightShadowFrag
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
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blendStates = {},
            .colorAttachmentCount = 0,
            .renderArea = std::nullopt
        };
    }

    void PointLightShadowTraditionalOpaqueAlphaTestedPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto& shadowGroup = drawData->PointLightShadow;
        auto fIdx = context.frameIndex;

        VkExtent2D extent = { POINT_SHADOW_ATLAS_SIZE, POINT_SHADOW_ATLAS_SIZE };
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

    void PointLightShadowTraditionalOpaqueAlphaTestedPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<PointLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowTraditionalOpaqueAlphaTestedPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto bindlessBuffer = imageManager->GetBindlessBuffer();
        bindlessBuffer->Bind(context.cmd, _shaderProgram->GetLayout(), 0, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void PointLightShadowTraditionalOpaqueAlphaTestedPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->PointLightShadow.indirectBuffer.GetHandle(context.frameIndex);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex);

        uint32_t commandOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.traditionalCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize indirectOffset = commandOffset * sizeof(VkDrawIndirectCommand);
            VkDeviceSize countOffset = _renderType * sizeof(uint32_t);

            vkCmdDrawIndirectCount(
                context.cmd,
                indirectBuffer,
                indirectOffset,
                countBuffer,
                countOffset,
                maxCommandCount,
                sizeof(VkDrawIndirectCommand)
            );
        }
    }
}