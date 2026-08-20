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

#include "PointLightShadowMeshletOpaqueAlphaTestedPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
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
#include "Engine/Scene/DrawData/PointLightShadowDrawGroup.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Video/VideoManager.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/PointLightShadowTraditionalMeshletPassPC.glsl"

    bool PointLightShadowMeshletOpaqueAlphaTestedPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightShadowComponent>();
        return pool && pool->Size() > 0;
    }

    PointLightShadowMeshletOpaqueAlphaTestedPass::PointLightShadowMeshletOpaqueAlphaTestedPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "PointLightShadowMeshletOpaqueAlphaTestedPass1Sided";
        }
        else {
            _passName = "PointLightShadowMeshletOpaqueAlphaTestedPass2Sided";
        }
    }

    void PointLightShadowMeshletOpaqueAlphaTestedPass::Initialize() {
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


        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowMeshletAlphaTestedProgram", {
            ShaderNames::PointLightShadowMeshletTask,
            ShaderNames::PointLightShadowMeshletMesh,
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

    void PointLightShadowMeshletOpaqueAlphaTestedPass::PrepareFrame(const RenderContext& context) {
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

    void PointLightShadowMeshletOpaqueAlphaTestedPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto drawData = scene->GetSceneDrawData();

        Vk::PushConstant<PointLightShadowTraditionalMeshletPassPC> pc{};
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.activeTraditionalCount + drawData->Models.meshletCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc->disableConeCulling = _renderType == MaterialRenderType::Opaque2Sided ? 1 : 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowMeshletOpaqueAlphaTestedPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        auto depthPyramid = context.scene->GetSceneDrawData()->PointLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        // pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);

        auto videoManager = ServiceLocator::Get<VideoManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;

        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();
        if (auto descBuffer = descriptorManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, descBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void PointLightShadowMeshletOpaqueAlphaTestedPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->PointLightShadow.indirectBuffer.GetHandle(context.frameIndex);
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