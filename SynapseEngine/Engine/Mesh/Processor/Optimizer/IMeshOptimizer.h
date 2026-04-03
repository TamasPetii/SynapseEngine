#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API IMeshOptimizer : public IMeshProcessor
    {
    public:
        virtual ~IMeshOptimizer() override = default;
    };
}