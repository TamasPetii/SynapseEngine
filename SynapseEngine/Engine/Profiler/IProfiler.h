#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <unordered_map>
#include <format>

namespace Syn {

    class SYN_API IProfiler {
    public:
        virtual ~IProfiler() = default;

        virtual void ResolveFrame(uint32_t frameIndex) = 0;
        virtual const std::unordered_map<std::string, float>& GetTimings(uint32_t frameIndex) const = 0;

        virtual std::string GenerateReport(uint32_t frameIndex, const std::string& title) const {
            const auto& rawTimings = GetTimings(frameIndex);
            if (rawTimings.empty()) {
                return "";
            }

            std::unordered_map<std::string, float> timings(rawTimings.begin(), rawTimings.end());

            struct PassGroup {
                std::string name;
                std::vector<std::string> passes;
            };

            std::vector<PassGroup> groups = {
                {"Setup Passes", {
                    "GlobalFrameSetupPass", "OpaqueInitPass", "TransparentInitPass", "HizInitPass"
                }},
                {"Geometry Culling", {
                    "CullingCommandResetPass", "ModelCullingPass", "MeshCullingPass"
                }},
                {"Depth Prepass (Opaque)", {
                    "OpaqueDepthTransitionPrepass", "MeshletOpaqueDepthPrepass1Sided",
                    "MeshletOpaqueDepthPrepass2Sided", "TraditionalOpaqueZPrepass1Sided",
                    "TraditionalOpaqueZPrepass2Sided", "Depth_Copy_Pass"
                }},
                {"Depth Prepass (Transparent)", {
                    "TransparentDepthTransitionPrepass", "MeshletTransparentDepthPrepass1Sided",
                    "MeshletTransparentDepthPrepass2Sided", "TraditionalTransparentDepthPrepass1Sided",
                    "TraditionalTransparentDepthPrepass2Sided"
                }},
                {"Hi-Z Generation", {
                    "HizLinearPreparePass", "HizDownsamplePass"
                }},
                {"Light Culling", {
                    "PointLightCullingPass", "SpotLightCullingPass"
                }},
                {"Forward+ Clustering", {
                    "ClusterSetupPass", "ClusterPointLightCountPass", "ClusterSpotLightCountPass",
                    "ClusterPrefixSumPass", "ClusterPointLightWritePass", "ClusterSpotLightWritePass",
                    "ClusterLightWriteSyncPass"
                }},
                {"Lighting (Opaque)", {
                    "OpaqueForwardTransitionPass", "MeshletOpaqueForward1Sided",
                    "MeshletOpaqueForward2Sided", "TraditionalOpaqueForward1Sided",
                    "TraditionalOpaqueForward2Sided"
                }},
                {"Lighting (Transparent WBOIT)", {
                    "TransparentForwardTransitionPass", "Meshlet_Transparent_Forward_1Sided",
                    "Meshlet_Transparent_Forward_2Sided", "Traditional_Transparent_Forward_1Sided",
                    "Traditional_Transparent_Forward_2Sided", "TransparentCompositeTransitionPass",
                    "Transparent_Composite"
                }},
                {"Bloom Post-Processing", {
                    "BloomPrefilterPass", "BloomDownsamplePass", "BloomUpsamplePass", "BloomCompositePass"
                }},
                {"Presentation & UI", {
                    "PresentationTransitionPass", "GuiPass"
                }}
            };

            std::string report = std::format("{} Timings:\n", title);
            float totalGpuTime = 0.0f;

            for (const auto& group : groups) {
                bool groupHasPass = false;
                std::string groupLines = "";

                for (const auto& passName : group.passes) {
                    auto it = timings.find(passName);
                    if (it != timings.end()) {
                        groupHasPass = true;
                        groupLines += std::format("    |   {:<42} : {:>8.3f} ms\n", passName, it->second);
                        totalGpuTime += it->second;
                        timings.erase(it);
                    }
                }

                if (groupHasPass) {
                    report += std::format("    +---[ {} ]\n", group.name);
                    report += groupLines;
                }
            }

            if (!timings.empty()) {
                report += "    +---[ Uncategorized Passes ]\n";
                for (const auto& [name, ms] : timings) {
                    report += std::format("    |   {:<42} : {:>8.3f} ms\n", name, ms);
                    totalGpuTime += ms;
                }
            }

            report += "    ----------------------------------------------------------------------\n";
            report += std::format("    = {:<44} : {:>8.3f} ms\n", "TOTAL " + title + " TIME", totalGpuTime);

            return report;
        }
    };

}