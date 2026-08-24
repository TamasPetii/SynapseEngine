#pragma once
#include "Engine/SynApi.h"
#include "IGpuProceduralPass.h"
#include "Engine/Vk/Shader/ShaderProgram.h"

namespace Syn {
    class SYN_API GpuComputePass : public IGpuProceduralPass {
    public:
        void Execute(const GpuProceduralContext& context) override;
    protected:
        virtual void BindDescriptors(const GpuProceduralContext& context) {}
        virtual void PushConstants(const GpuProceduralContext& context) {}
        virtual void Dispatch(const GpuProceduralContext& context) {}
    protected:
        uint32_t _shaderProgramId = UINT32_MAX;
        Vk::ShaderProgram* _shaderProgram = nullptr;
    };
}