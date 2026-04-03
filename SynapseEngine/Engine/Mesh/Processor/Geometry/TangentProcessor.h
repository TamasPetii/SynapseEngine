#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API TangentProcessor : public IMeshProcessor
    {
    public:
        virtual ~TangentProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    };
}