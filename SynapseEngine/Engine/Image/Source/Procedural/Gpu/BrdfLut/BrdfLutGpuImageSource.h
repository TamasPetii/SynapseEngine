#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Source/Procedural/ProceduralImageSource.h"
#include "Engine/Image/Source/Procedural/Pipeline/GpuProceduralPipeline.h"
#include <memory>

namespace Syn
{
    class SYN_API BrdfLutGpuImageSource : public ProceduralImageSource
    {
    public:
        BrdfLutGpuImageSource(uint32_t resolution = 512, uint32_t sampleCount = 1024);
        ~BrdfLutGpuImageSource() override = default;

        std::optional<RawImage> Produce() override;
    private:
        uint32_t _resolution;
        uint32_t _sampleCount;
        std::shared_ptr<GpuProceduralPipeline> _pipeline;
    };
}