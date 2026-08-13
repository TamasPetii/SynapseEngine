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