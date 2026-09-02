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

#include "SwapchainPresentPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Render/RenderNames.h"

namespace Syn 
{
    void SwapchainPresentPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::Get<Vk::Context>();
        auto swapchainImage = vkContext->GetSwapChain()->GetImage(context.swapchainImageIndex);

        _imageTransitions.push_back({
            .image = swapchainImage,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .dstStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .dstAccess = VK_ACCESS_2_NONE,
            .discardContent = false
            });
    }
}