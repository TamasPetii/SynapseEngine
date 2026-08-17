#pragma once
#include "Engine/SynApi.h"
#include "IEnvironmentBaker.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <string>

namespace Syn {
    class SYN_API EnvironmentComputeBaker : public IEnvironmentBaker {
    public:
        void Bake(const EnvironmentBakeContext& context) override;

    protected:
        virtual void BindDescriptors(const EnvironmentBakeContext& context) {}
        virtual void PushConstants(const EnvironmentBakeContext& context) {}
        virtual void Dispatch(const EnvironmentBakeContext& context) {}
    protected:
        uint32_t _shaderProgramId = UINT32_MAX;
        Vk::ShaderProgram* _shaderProgram = nullptr;
    };
}