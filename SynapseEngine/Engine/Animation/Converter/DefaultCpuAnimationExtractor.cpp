#include "DefaultCpuAnimationExtractor.h"
#include <utility>

namespace Syn
{
    void DefaultCpuAnimationExtractor::Extract(GpuBatchedAnimation& gpuData, CpuAnimationData& outCpuData) const
    {
		outCpuData.descriptor = gpuData.descriptor;
		outCpuData.globalCollider = gpuData.globalCollider;

        outCpuData.frameGlobalColliders = std::move(gpuData.frameGlobalColliders);
        outCpuData.frameMeshColliders = std::move(gpuData.frameMeshColliders);
        outCpuData.tracks = std::move(gpuData.tracks);
    }
}