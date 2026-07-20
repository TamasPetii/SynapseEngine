#pragma once
#include "Engine/SynApi.h"
#include "IRenderStatCollector.h"
#include "Engine/Vk/Query/PipelineStatisticsQueryPool.h"

#include <vector>
#include <memory>
#include <string>

namespace Syn {

    struct SYN_API StatMeasurement {
        std::string groupName;
        std::string passName;
        uint32_t queryIndex;
    };

    class SYN_API DefaultRenderStatCollector : public IRenderStatCollector {
    public:
        explicit DefaultRenderStatCollector(uint32_t framesInFlight);
        ~DefaultRenderStatCollector() override = default;

        DefaultRenderStatCollector(const DefaultRenderStatCollector&) = delete;
        DefaultRenderStatCollector& operator=(const DefaultRenderStatCollector&) = delete;

        void BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) override;
        uint32_t StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) override;
        void EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t queryIndex) override;

        void ResolveFrame(uint32_t frameIndex) override;
        const std::vector<RenderPassStats>& GetStats(uint32_t frameIndex) const override;
    private:
        static constexpr uint32_t MAX_QUERIES_PER_FRAME = 1024;

        uint32_t _framesInFlight;
        std::vector<uint32_t> _queryCounters;
        std::vector<std::unique_ptr<Vk::PipelineStatisticsQueryPool>> _pools;

        std::vector<std::vector<StatMeasurement>> _activeMeasurements;
        std::vector<std::vector<RenderPassStats>> _resolvedStats;
    };
}