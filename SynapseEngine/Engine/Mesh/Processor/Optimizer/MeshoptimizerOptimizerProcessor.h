#pragma once
#include "Engine/SynApi.h"
#include "IMeshOptimizer.h" // Vagy az az interface, amiből a többiek is származnak

namespace Syn
{
    class SYN_API MeshoptimizerOptimizerProcessor : public IMeshOptimizer
    {
    public:
        MeshoptimizerOptimizerProcessor(float overdrawThreshold = 1.05f);
        virtual ~MeshoptimizerOptimizerProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    private:
        float _overdrawThreshold;
    };
}