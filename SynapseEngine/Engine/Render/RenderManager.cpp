#include "RenderManager.h"

#include "Engine/Vk/Command/CommandBuffer.h"

namespace Syn {

    RenderManager::RenderManager(uint32_t framesInFlight) {
        _renderer = std::make_unique<Renderer>(framesInFlight);
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

    void RenderManager::RenderFrame(uint32_t frameIndex, std::shared_ptr<Scene> scene) {
        if (!_activePipeline) 
            return;

        auto cmd = _renderer->BeginFrame(frameIndex);

        if (!cmd) 
            return;

        RenderContext context = {
            .cmd = cmd->Handle(),
            .frameIndex = frameIndex,
			.swapchainImageIndex = _renderer->GetCurrentImageIndex(),
            .scene = scene,
        };

        _activePipeline->Execute(context);

        _renderer->EndFrame(frameIndex);
    }
}