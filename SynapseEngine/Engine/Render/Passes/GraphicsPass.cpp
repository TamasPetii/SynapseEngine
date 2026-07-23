#include "GraphicsPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"

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