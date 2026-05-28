#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API NormalProcessor : public IMeshProcessor
    {
    public:
        virtual ~NormalProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    };
}