// (C)2026 Tamás Péter
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
#include "Engine/Render/Passes/ComputePass.h"
#include "Engine/Vk/Image/Image.h"
#include <vector>

namespace Syn
{
    class SYN_API VideoConversionPass : public ComputePass {
    public:
        std::string GetName() const override { return "VideoConversionPass"; }
        std::string GetGroup() const override { return PassGroupNames::InitSetupPasses; }
        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void PrepareFrame(const RenderContext& context) override;
        void Dispatch(const RenderContext& context) override;
    private:
        struct ActiveVideoTarget {
            std::shared_ptr<Vk::Image> sourceYuv;
            std::shared_ptr<Vk::Image> targetRgba;
            VkImageView lumaView = VK_NULL_HANDLE;
            VkImageView chromaView = VK_NULL_HANDLE;
            uint32_t width;
            uint32_t height;
        };

        std::vector<ActiveVideoTarget> _activeTargets;
    };
}