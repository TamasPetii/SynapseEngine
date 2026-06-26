#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "Engine/Mesh/Data/Cooked/CookedModel.h"

namespace Syn
{
    class SYN_API IMeshProcessor
    {
    public:
        virtual ~IMeshProcessor() = default;
        virtual void Process(CookedModel& cookedModel) = 0;
    };
}