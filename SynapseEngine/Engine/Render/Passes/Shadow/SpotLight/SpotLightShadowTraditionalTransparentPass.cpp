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

#include "SpotLightShadowTraditionalTransparentPass.h"
#include "Engine/ServiceLocator.h"
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
#include <cassert>

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/SpotLightShadowTraditionalMeshletPassPC.glsl"

    SpotLightShadowTraditionalTransparentPass::SpotLightShadowTraditionalTransparentPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        switch (_renderType) {
        case MaterialRenderType::Transparent1Sided:
            _passName = "SpotLightShadowTraditionalTransparentPass1Sided";
            break;
        case MaterialRenderType::Transparent2Sided:
            _passName = "SpotLightShadowTraditionalTransparentPass2Sided";
            break;
        case MaterialRenderType::AlphaTestedTransparent1Sided:
            _passName = "SpotLightShadowTraditionalAlphaTestedTransparentPass1Sided";
            break;
        case MaterialRenderType::AlphaTestedTransparent2Sided:
            _passName = "SpotLightShadowTraditionalAlphaTestedTransparentPass2Sided";
            break;
        default:
            assert(false);
            break;
        }
    }

    bool SpotLightShadowTraditionalTransparentPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    void SpotLightShadowTraditionalTransparentPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.defines = { ShaderDefines::EnableAlphaTest };
        config.layoutOverride = [descriptorManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return descriptorManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };

        _shaderProgramId = shaderManager->LoadProgramAsync("SpotLightShadowTraditionalTransparentProgram", {
            ShaderNames::SpotLightShadowTraditionalVert,
            ShaderNames::SpotLightShadowTransparentFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::Transparent2Sided ||
            _renderType == MaterialRenderType::AlphaTestedTransparent2Sided)
            ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

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
                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorFactor = VK_BLEND_FACTOR_SRC_COLOR,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_MIN
                }
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void SpotLightShadowTraditionalTransparentPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto& shadowGroup = drawData->SpotLightShadow;
        auto fIdx = context.frameIndex;

        VkExtent2D extent = { SPOT_SHADOW_ATLAS_SIZE, SPOT_SHADOW_ATLAS_SIZE };
        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = shadowGroup.shadowColorAtlas[fIdx]->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = shadowGroup.shadowAtlas[fIdx]->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
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

    void SpotLightShadowTraditionalTransparentPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<SpotLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowTraditionalTransparentPass::BindDescriptors(const RenderContext& context)
    {
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;

        if (auto descBuffer = descriptorManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, descBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void SpotLightShadowTraditionalTransparentPass::Draw(const RenderContext& context)
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