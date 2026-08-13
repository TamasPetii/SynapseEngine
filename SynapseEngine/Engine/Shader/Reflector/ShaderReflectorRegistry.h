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

#pragma once
#include "Engine/SynApi.h"
#include "IShaderReflectorRegistry.h"
#include <unordered_map>

namespace Syn {
    class SYN_API ShaderReflectorRegistry : public IShaderReflectorRegistry {
    public:
        ShaderReflectorRegistry() = default;
        ~ShaderReflectorRegistry() override = default;

        void Register(ShaderBytecodeFormat format, std::shared_ptr<IShaderReflector> reflector) override;
        IShaderReflector* GetReflector(ShaderBytecodeFormat format) override;

    private:
        std::unordered_map<ShaderBytecodeFormat, std::shared_ptr<IShaderReflector>> _reflectors;
    };
}