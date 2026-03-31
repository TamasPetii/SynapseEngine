#include "TransferPass.h"

namespace Syn {
    void TransferPass::Execute(const RenderContext& context)
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

        Transfer(context);
    }
}