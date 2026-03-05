#include "BaseRenderPass.h"

namespace Syn {
    void BaseRenderPass::Execute(const RenderContext& context)
    {
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