// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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