#include "ComputePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"

namespace Syn {
    void ComputePass::Execute(const RenderContext& context)
    {
        if (!_shaderProgram && _shaderProgramId != UINT32_MAX)
            _shaderProgram = ServiceLocator::Get<ShaderManager>()->GetResource(_shaderProgramId).get();

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