#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Source/Procedural/Pipeline/GpuComputePass.h"

namespace Syn
{
    class SYN_API BrdfLutComputePass : public GpuComputePass {
    public:
        BrdfLutComputePass(uint32_t sampleCount);
        void Initialize() override;
    protected:
        void BindDescriptors(const GpuProceduralContext& context) override;
        void PushConstants(const GpuProceduralContext& context) override;
        void Dispatch(const GpuProceduralContext& context) override;
    private:
        uint32_t _sampleCount;
    };
}