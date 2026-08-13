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
#include "Engine/Vk/Image/Image.h"

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

#include "RenderContext.h"
#include "ShaderNames.h"
#include "RenderNames.h"
#include "PassGroupNames.h"

namespace Syn 
{
    struct SYN_API PassImageTransition
    {
        Vk::Image* image;
        VkImageLayout newLayout;
        VkPipelineStageFlags2 dstStage;
        VkAccessFlags2 dstAccess;
        bool discardContent = false;
    };

    class SYN_API IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;
        virtual void Initialize() {};
        virtual void Execute(const RenderContext& context) {};
        virtual bool ShouldExecute(const RenderContext& context) const { return true; }
        virtual bool ShouldCollectStatistics() const { return false; }
        virtual std::string GetName() const = 0;
        virtual std::string GetGroup() const { return PassGroupNames::UndefinedPasses; }
    };
}

