#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/RenderManager.h"
#include <memory>

namespace Syn {
    class SYN_API RendererFactory {
    public:
        static std::unique_ptr<RenderManager> CreateDeferredRenderer(uint32_t framesInFlight);
        static std::unique_ptr<RenderManager> CreatePerformanceRenderer(uint32_t framesInFlight);
    };

}