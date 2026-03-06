#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"

namespace Syn
{
    class SYN_API IModelCooker
    {
    public:
        virtual ~IModelCooker() = default;
        virtual CookedModel Cook(RawModel&& rawModel) = 0;
    };
}