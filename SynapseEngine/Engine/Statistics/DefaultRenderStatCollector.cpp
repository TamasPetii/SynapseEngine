#include "DefaultRenderStatCollector.h"

namespace Syn {

    DefaultRenderStatCollector::DefaultRenderStatCollector(uint32_t framesInFlight)
        : _framesInFlight(framesInFlight)
    {
        _pools.resize(framesInFlight);
        _activeMeasurements.resize(framesInFlight);
        _resolvedStats.resize(framesInFlight);
        _queryCounters.resize(framesInFlight, 0);

        VkQueryPipelineStatisticFlags flags =
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
            VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT /* |
            VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | 
            VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT */;

        for (uint32_t i = 0; i < framesInFlight; ++i) {
            _pools[i] = std::make_unique<Vk::PipelineStatisticsQueryPool>(MAX_QUERIES_PER_FRAME, flags);
        }
    }

    void DefaultRenderStatCollector::BeginFrame(VkCommandBuffer cmd, uint32_t frameIndex) {
        _queryCounters[frameIndex] = 0;
        _activeMeasurements[frameIndex].clear();
        _pools[frameIndex]->Reset(cmd, 0, MAX_QUERIES_PER_FRAME);
    }

    uint32_t DefaultRenderStatCollector::StartPass(VkCommandBuffer cmd, uint32_t frameIndex, const std::string& groupName, const std::string& name) {
        uint32_t queryIndex = _queryCounters[frameIndex]++;

        _pools[frameIndex]->BeginQuery(cmd, queryIndex);
        _activeMeasurements[frameIndex].push_back({ groupName, name, queryIndex });

        return queryIndex;
    }

    void DefaultRenderStatCollector::EndPass(VkCommandBuffer cmd, uint32_t frameIndex, uint32_t queryIndex) {
        _pools[frameIndex]->EndQuery(cmd, queryIndex);
    }

    void DefaultRenderStatCollector::ResolveFrame(uint32_t frameIndex) {
        auto& measurements = _activeMeasurements[frameIndex];
        if (measurements.empty()) return;

        uint32_t totalQueries = _queryCounters[frameIndex];
        std::vector<uint64_t> results(totalQueries * STATS_PER_QUERY);

        if (_pools[frameIndex]->GetResults(0, totalQueries, results, false)) {
            auto& stats = _resolvedStats[frameIndex];
            stats.clear();
            stats.reserve(measurements.size());

            for (size_t i = 0; i < measurements.size(); ++i) {
                const auto& m = measurements[i];

                //uint32_t offset = m.queryIndex * STATS_PER_QUERY;
                uint32_t offset = m.queryIndex * 6;

                RenderPassStats passStat;
                passStat.groupName = m.groupName;
                passStat.passName = m.passName;
                 
                passStat.inputAssemblyVertices = results[offset + 0];
                passStat.inputAssemblyPrimitives = results[offset + 1];
                passStat.vertexShaderInvocations = results[offset + 2];
                passStat.clippingInvocations = results[offset + 3];
                passStat.clippingPrimitives = results[offset + 4];
                passStat.fragmentShaderInvocations = results[offset + 5];
                //passStat.taskShaderInvocations = results[offset + 6];
                //passStat.meshShaderInvocations = results[offset + 7];

                stats.push_back(passStat);
            }
        }
    }

    const std::vector<RenderPassStats>& DefaultRenderStatCollector::GetStats(uint32_t frameIndex) const {
        return _resolvedStats[frameIndex];
    }
}