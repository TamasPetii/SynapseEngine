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
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <string>
#include <vector>
#include <memory>

#include "../IRenderPass.h"

namespace Syn
{
    class SYN_API TransitionPass : public IRenderPass
    {
    public:
        void Execute(const RenderContext& context) override;
    protected:
        virtual void PrepareFrame(const RenderContext& context) {}
        virtual void Barrier(const RenderContext& context) {}
    protected:
        std::vector<PassImageTransition> _imageTransitions;
    };
}