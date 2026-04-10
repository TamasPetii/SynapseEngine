#pragma once
#include "Engine/SynApi.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

#include "Renderer.h"
#include "RenderPipeline.h"
#include "Engine/Scene/Scene.h"
#include "RenderTargetManager.h"

namespace Syn {
    class SYN_API RenderManager {
    public:
        explicit RenderManager(uint32_t framesInFlight);

        RenderManager(const RenderManager&) = delete;
        RenderManager& operator=(const RenderManager&) = delete;

        void RegisterPipeline(const std::string& name, std::unique_ptr<RenderPipeline> pipeline);
        void SetActivePipeline(const std::string& name);

        void WaitForFrame(uint32_t frameIndex);
        void RenderFrame(uint32_t frameIndex, uint32_t framesInFlight, Scene* scene);
        void OnResize(uint32_t width, uint32_t height);

        RenderTargetManager* GetRenderTargetManager() const { return _renderTargetManager.get(); }

        void SetGuiRenderCallback(std::function<void(VkCommandBuffer)> callback) {
            _onRenderGuiCallback = std::move(callback);
        }

        void SetPreRenderCallback(std::function<void(VkCommandBuffer, uint32_t, Scene*)> callback) {
            _preRenderCallback = std::move(callback);
        }

        bool IsResizePending() const {
            for (bool needsResize : _frameNeedsResize) {
                if (needsResize) return true;
            }
            return false;
        }
    private:
        std::unique_ptr<Renderer> _renderer;
        std::unique_ptr<RenderTargetManager> _renderTargetManager;
        std::unordered_map<std::string, std::unique_ptr<RenderPipeline>> _pipelines;
        RenderPipeline* _activePipeline = nullptr;

        std::vector<bool> _frameNeedsResize;
        uint32_t _newWidth = 0;
        uint32_t _newHeight = 0;

        std::function<void(VkCommandBuffer)> _onRenderGuiCallback;
        std::function<void(VkCommandBuffer, uint32_t, Scene*)> _preRenderCallback;
    };
}