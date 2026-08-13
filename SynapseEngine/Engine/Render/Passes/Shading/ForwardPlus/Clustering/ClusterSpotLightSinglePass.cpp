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

#include "ClusterSpotLightSinglePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {
#include "Engine/Shaders/Includes/PushConstants/ClusterLightWritePC.glsl"

    void ClusterSpotLightSinglePass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ClusterSpotLightSingleProgram", {
            ShaderNames::ClusterSpotLightSingle
            }, config);
    }

    void ClusterSpotLightSinglePass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ClusterLightWritePC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ClusterSpotLightSinglePass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        VkBuffer indirectBuffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(context.frameIndex);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, offsetof(ForwardPlusDispatchArgs, spotFastPath));
    }
}