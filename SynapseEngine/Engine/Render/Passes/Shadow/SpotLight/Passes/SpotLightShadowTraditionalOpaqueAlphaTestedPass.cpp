// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "SpotLightShadowTraditionalOpaqueAlphaTestedPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Scene/DrawData/SpotLightShadowDrawGroup.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Video/VideoManager.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/SpotLightShadowTraditionalMeshletPassPC.glsl"

    bool SpotLightShadowTraditionalOpaqueAlphaTestedPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    SpotLightShadowTraditionalOpaqueAlphaTestedPass::SpotLightShadowTraditionalOpaqueAlphaTestedPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "SpotLightShadowTraditionalOpaqueAlphaTestedPass1Sided";
        }
        else {
            _passName = "SpotLightShadowTraditionalOpaqueAlphaTestedPass2Sided";
        }
    }

    void SpotLightShadowTraditionalOpaqueAlphaTestedPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto videoManager = ServiceLocator::Get<VideoManager>();
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();

        Vk::ShaderProgramConfig config;
        config.defines = { ShaderDefines::EnableAlphaTest };
        config.layoutOverride = [descriptorManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return descriptorManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };


        _shaderProgramId = shaderManager->LoadProgramAsync("SpotLightShadowAlphaTestedProgram", {
            ShaderNames::SpotLightShadowTraditionalVert,
            ShaderNames::SpotLightShadowFrag
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

    void SpotLightShadowTraditionalOpaqueAlphaTestedPass::PrepareFrame(const RenderContext& context) {
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

    void SpotLightShadowTraditionalOpaqueAlphaTestedPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        if (!scene) return;

        uint32_t fIdx = context.frameIndex;

        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<SpotLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowTraditionalOpaqueAlphaTestedPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto videoManager = ServiceLocator::Get<VideoManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;

        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();
        if (auto descBuffer = descriptorManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, descBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void SpotLightShadowTraditionalOpaqueAlphaTestedPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->SpotLightShadow.indirectBuffer.GetHandle(context.frameIndex);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex);

        uint32_t commandOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.traditionalCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize indirectOffset = commandOffset * sizeof(VkDrawIndirectCommand);
            VkDeviceSize countBufferOffset = _renderType * sizeof(uint32_t);

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