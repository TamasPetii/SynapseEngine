#include "DefaultRenderStatCollector.h"
#include <bit>

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
            VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
            VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT | 
            VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;

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
		auto activeFlags = _pools[frameIndex]->GetFlags();
        uint32_t activeStatCount = std::popcount(static_cast<uint32_t>(activeFlags));
        std::vector<uint64_t> results(totalQueries * activeStatCount);

        if (_pools[frameIndex]->GetResults(0, totalQueries, results, false)) {
            auto& stats = _resolvedStats[frameIndex];
            stats.clear();
            stats.reserve(measurements.size());

            for (size_t i = 0; i < measurements.size(); ++i) {
                const auto& m = measurements[i];

                RenderPassStats passStat;
                passStat.groupName = m.groupName;
                passStat.passName = m.passName;
                 
                uint32_t localIndex = 0;
                uint32_t offset = m.queryIndex * activeStatCount;

                auto extractStat = [&](VkQueryPipelineStatisticFlags flag) -> uint64_t {
                    return (activeFlags & flag) ? results[offset + localIndex++] : 0;
                    };

                passStat.inputAssemblyVertices = extractStat(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT);
                passStat.inputAssemblyPrimitives = extractStat(VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT);
                passStat.vertexShaderInvocations = extractStat(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT);
                passStat.clippingInvocations = extractStat(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT);
                passStat.clippingPrimitives = extractStat(VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT);
                passStat.fragmentShaderInvocations = extractStat(VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT);
                passStat.taskShaderInvocations = extractStat(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);
                passStat.meshShaderInvocations = extractStat(VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT);

                stats.push_back(passStat);
            }
        }
    }

    const std::vector<RenderPassStats>& DefaultRenderStatCollector::GetStats(uint32_t frameIndex) const {
        return _resolvedStats[frameIndex];
    }
}