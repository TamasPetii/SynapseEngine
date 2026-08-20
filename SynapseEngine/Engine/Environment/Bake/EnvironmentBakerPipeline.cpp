#include "EnvironmentBakerPipeline.h"

namespace Syn {
    void EnvironmentBakerPipeline::AddBaker(std::unique_ptr<IEnvironmentBaker> baker) {
        _bakers.push_back(std::move(baker));
    }

    void EnvironmentBakerPipeline::InitializeAll() {
        for (auto& baker : _bakers) {
            baker->Initialize();
        }
    }

    void EnvironmentBakerPipeline::Execute(const EnvironmentBakeContext& context) {
        for (auto& baker : _bakers) {
            baker->Bake(context);
        }
    }
}