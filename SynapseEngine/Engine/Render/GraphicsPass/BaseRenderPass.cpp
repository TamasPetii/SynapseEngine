#include "BaseRenderPass.h"

namespace Syn {
    void BaseRenderPass::Execute(VkCommandBuffer cmd, const RenderScene& scene)
    {
        for (const auto& transition : _imageTransitions) {
            transition.image->TransitionLayout(
                cmd,
                transition.newLayout,
                transition.dstStage,
                transition.dstAccess,
                transition.discardContent
            );
        }

        if (_useDynamicRendering && _renderInfo.has_value()) {
            Vk::RenderUtils::BeginRendering(cmd, _renderInfo.value());
        }

        if (_shaderProgram) {
            Vk::RenderUtils::SetGraphicsState(cmd, _graphicsState);
            _shaderProgram->Bind(cmd);

            BindDescriptors(cmd, scene);
            PushConstants(cmd, scene);
            Draw(cmd, scene);
        }

        if (_useDynamicRendering) {
            Vk::RenderUtils::EndRendering(cmd);
        }
    }
}