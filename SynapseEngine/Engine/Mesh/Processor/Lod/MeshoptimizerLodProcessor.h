#pragma once
#include "Engine/SynApi.h"
#include "ILodProcessor.h"

namespace Syn
{
    class SYN_API MeshoptimizerLodProcessor : public ILodProcessor
    {
    public:
        MeshoptimizerLodProcessor(uint32_t maxLods = 4, float targetError = 0.05f);
        virtual ~MeshoptimizerLodProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    private:
        uint32_t _maxLods;
        float _targetError;
    };
}