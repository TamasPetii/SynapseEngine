#pragma once
#include "Engine/SynApi.h"
#include "IEnvironmentBaker.h"
#include <vector>
#include <memory>

namespace Syn {
    class SYN_API EnvironmentBakerPipeline {
    public:
        EnvironmentBakerPipeline() = default;
        EnvironmentBakerPipeline(const EnvironmentBakerPipeline&) = delete;
        EnvironmentBakerPipeline& operator=(const EnvironmentBakerPipeline&) = delete;

        void AddBaker(std::unique_ptr<IEnvironmentBaker> baker);
        void InitializeAll();
        void Execute(const EnvironmentBakeContext& context);

    private:
        std::vector<std::unique_ptr<IEnvironmentBaker>> _bakers;
    };
}