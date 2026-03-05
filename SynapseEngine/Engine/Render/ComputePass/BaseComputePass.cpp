#include "BaseComputePass.h"

namespace Syn {
    void BaseComputePass::Execute(const RenderContext& context)
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

        if (_shaderProgram) {
            _shaderProgram->Bind(context.cmd);

            BindDescriptors(context);
            PushConstants(context);
            Dispatch(context);
        }
    }
}