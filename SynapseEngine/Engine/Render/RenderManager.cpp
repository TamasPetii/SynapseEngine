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

#include "RenderManager.h"

#include "Engine/Vk/Command/CommandBuffer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/FrameContext.h"

namespace Syn {

    RenderManager::RenderManager(uint32_t framesInFlight) {
        _renderer = std::make_unique<Renderer>(framesInFlight);
        _renderTargetManager = std::make_unique<RenderTargetManager>(framesInFlight);
        _frameNeedsResize.resize(framesInFlight, false);
    }

    void RenderManager::RegisterPipeline(const std::string& name, std::unique_ptr<RenderPipeline> pipeline) {
        SYN_ASSERT(pipeline != nullptr, "Cannot register a null pipeline!");

        _pipelines[name] = std::move(pipeline);

        if (_activePipeline == nullptr) {
            _activePipeline = _pipelines[name].get();
        }
    }

    void RenderManager::SetActivePipeline(const std::string& name) {
        auto it = _pipelines.find(name);
        SYN_ASSERT(it != _pipelines.end(), "Pipeline not found!");
        _activePipeline = it->second.get();
    }

    void RenderManager::WaitForFrame(uint32_t frameIndex) {
        _renderer->WaitForFrame(frameIndex);
    }

    void RenderManager::RenderFrame(uint32_t frameIndex, uint32_t framesInFlight, Scene* scene) {
        if (!_activePipeline) 
            return;

        if (_frameNeedsResize[frameIndex]) 
        {
            ServiceLocator::Get<Vk::Context>()->GetDevice()->WaitIdle();
            _renderTargetManager->Resize(frameIndex, _newWidth, _newHeight);
            _frameNeedsResize[frameIndex] = false;
        }
        
        auto cmd = _renderer->BeginFrame(frameIndex);

        if (!cmd) 
            return;

        if (_preRenderCallback) {
            _preRenderCallback(cmd->Handle(), frameIndex, scene);
        }

        if (_preRenderCallback) {
            _preRenderCallback(cmd->Handle(), frameIndex, scene);
        }

        RenderContext context = {
            .cmd = cmd->Handle(),
            .frameIndex = frameIndex,
            .framesInFlight = framesInFlight,
			.swapchainImageIndex = _renderer->GetCurrentImageIndex(),
            .scene = scene,
            .renderTargetManager = _renderTargetManager.get(),
            .onRenderGui = _onRenderGuiCallback
        };

        _activePipeline->Execute(context);

        _renderer->EndFrame(frameIndex);
    }

    void RenderManager::OnResize(uint32_t width, uint32_t height)
    {
        _newWidth = width;
        _newHeight = height;

        for (size_t i = 0; i < _frameNeedsResize.size(); ++i) {
            _frameNeedsResize[i] = true;
        }

        ServiceLocator::Get<FrameContext>()->screenWidth = width;
		ServiceLocator::Get<FrameContext>()->screenHeight = height;
    }
}