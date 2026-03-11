#pragma once
#include "Engine/SynApi.h"
#include <memory>
#include <string>
#include <unordered_map>

#include "Renderer.h"
#include "RenderPipeline.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    class SYN_API RenderManager {
    public:
        explicit RenderManager(uint32_t framesInFlight);

        RenderManager(const RenderManager&) = delete;
        RenderManager& operator=(const RenderManager&) = delete;

        void RegisterPipeline(const std::string& name, std::unique_ptr<RenderPipeline> pipeline);
        void SetActivePipeline(const std::string& name);

        void WaitForFrame(uint32_t frameIndex);
        void RenderFrame(uint32_t frameIndex, std::shared_ptr<Scene> scene);
    private:
        std::unique_ptr<Renderer> _renderer;
        std::unordered_map<std::string, std::unique_ptr<RenderPipeline>> _pipelines;
        RenderPipeline* _activePipeline = nullptr;
    };
}