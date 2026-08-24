#include "GpuComputePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"

namespace Syn {
    void GpuComputePass::Execute(const GpuProceduralContext& context) {
        if (!_shaderProgram && _shaderProgramId != UINT32_MAX) {
            _shaderProgram = ServiceLocator::Get<ShaderManager>()->GetResource(_shaderProgramId).get();
        }

        if (!_shaderProgram) return;

        _shaderProgram->Bind(context.cmd);
        PushConstants(context);
        BindDescriptors(context);
        Dispatch(context);
    }
}