#include "BaseComputePass.h"

namespace Syn {
    void BaseComputePass::Execute(VkCommandBuffer cmd, const RenderScene& scene)
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

        if (_shaderProgram) {
            _shaderProgram->Bind(cmd);

            BindDescriptors(cmd, scene);
            PushConstants(cmd, scene);
            Dispatch(cmd, scene);
        }
    }
}