#include "EnvironmentComputeBaker.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"

namespace Syn {
    void EnvironmentComputeBaker::Bake(const EnvironmentBakeContext& context) 
    {
        if (!_shaderProgram && _shaderProgramId != UINT32_MAX) {
            _shaderProgram = ServiceLocator::Get<ShaderManager>()->GetResource(_shaderProgramId).get();
        }

        if (_shaderProgram) {
            _shaderProgram->Bind(context.cmd);
            BindDescriptors(context);
            PushConstants(context);
            Dispatch(context);
        }
    }
}