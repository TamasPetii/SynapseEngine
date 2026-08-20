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

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "Engine/Manager/DescriptorManager.h"
#include <string>
#include <vector>
#include <memory>

#include "../IRenderPass.h"

namespace Syn
{
    class SYN_API GraphicsPass : public IRenderPass
    {
    public:
        virtual void Execute(const RenderContext& context) override;
    protected:
        virtual void PrepareFrame(const RenderContext& context) {}
        virtual void BindDescriptors(const RenderContext& context) {}
        virtual void PushConstants(const RenderContext& context) {}
        virtual void Draw(const RenderContext& context) {};
    protected:
        bool _useDynamicRendering = true;
        uint32_t _shaderProgramId = UINT32_MAX;
        Vk::ShaderProgram* _shaderProgram = nullptr;;
        Vk::GraphicsPipelineConfig _graphicsState;
        std::vector<PassImageTransition> _imageTransitions;
        std::vector<VkRenderingAttachmentInfo> _colorAttachments;
        std::optional<VkRenderingAttachmentInfo> _depthAttachment;
        std::optional<VkRenderingAttachmentInfo> _stencilAttachment;
        std::optional<Vk::RenderingInfoConfig> _renderInfo;
    };
}