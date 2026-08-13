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

#include "DefaultCpuShaderExtractor.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    DefaultCpuShaderExtractor::DefaultCpuShaderExtractor(std::shared_ptr<IShaderReflectorRegistry> reflectorRegistry)
        : _reflectorRegistry(std::move(reflectorRegistry))
    {}

    void DefaultCpuShaderExtractor::Extract(const GpuShaderData& gpuData, CpuShaderData& outCpuData) const {
        outCpuData.stage = gpuData.stage;

        if (gpuData.spirv.empty()) {
            Warning("GpuShaderData contains no SPIR-V data to reflect.");
            return;
        }

        IShaderReflector* reflector = _reflectorRegistry->GetReflector(ShaderBytecodeFormat::SPIRV);

        if (!reflector) {
            Error("No suitable reflector found for SPIR-V format.");
            return;
        }

        outCpuData.reflection = reflector->Reflect(gpuData.spirv, gpuData.stage, outCpuData.identifier);
    }
}