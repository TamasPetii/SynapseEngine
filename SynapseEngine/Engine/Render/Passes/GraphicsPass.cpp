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

#include "GraphicsPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"

namespace Syn {
    void GraphicsPass::Execute(const RenderContext& context)
    {
        if (!_shaderProgram && _shaderProgramId != UINT32_MAX)
            _shaderProgram = ServiceLocator::Get<ShaderManager>()->GetResource(_shaderProgramId).get();

        _colorAttachments.clear();
        _imageTransitions.clear();

        PrepareFrame(context);

        for (const auto& transition : _imageTransitions) {
            transition.image->TransitionLayout(
                context.cmd,
                transition.newLayout,
                transition.dstStage,
                transition.dstAccess,
                transition.discardContent
            );
        }

        if (_useDynamicRendering && _renderInfo.has_value()) {
            Vk::RenderUtils::BeginRendering(context.cmd, _renderInfo.value());
        }

        if (_shaderProgram) {
            Vk::RenderUtils::SetGraphicsState(context.cmd, _graphicsState);
            _shaderProgram->Bind(context.cmd);

            BindDescriptors(context);
            PushConstants(context);
            Draw(context);
        }

        if (_useDynamicRendering && _renderInfo.has_value()) {
            Vk::RenderUtils::EndRendering(context.cmd);
        }
    }
}