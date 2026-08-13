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

#include "DefaultGpuShaderConverter.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    DefaultGpuShaderConverter::DefaultGpuShaderConverter(std::shared_ptr<IShaderCompilerRegistry> compilerRegistry)
        : _compilerRegistry(std::move(compilerRegistry))
    {}

    GpuShaderData DefaultGpuShaderConverter::Convert(const CookedShader& cookedShader) {
        GpuShaderData gpuData;
        gpuData.stage = cookedShader.stage;

        IShaderCompiler* compiler = _compilerRegistry->GetCompiler(cookedShader.language);

        if (!compiler) {
            Error("No suitable compiler found for shader language in file: {}", cookedShader.identifier);
            return gpuData;
        }

        Info("Compiling shader to SPIR-V: {}", cookedShader.identifier);

        gpuData.spirv = compiler->Compile(cookedShader);

        return gpuData;
    }
}