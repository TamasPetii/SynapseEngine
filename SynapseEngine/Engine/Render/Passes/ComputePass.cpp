#include "ComputePass.h"

namespace Syn {
    void ComputePass::Execute(const RenderContext& context)
    {
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

        if (_shaderProgram) {
            _shaderProgram->Bind(context.cmd);

            BindDescriptors(context);
            PushConstants(context);
            Dispatch(context);
        }
    }
}