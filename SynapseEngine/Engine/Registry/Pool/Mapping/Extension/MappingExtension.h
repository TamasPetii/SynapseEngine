#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

namespace Syn
{
    template<typename PoolType>
    struct DefaultMappingExtension {};

    template<typename MappingPolicy>
    struct MappingTraits
    {
        template<typename PoolType>
        using Extension = DefaultMappingExtension<PoolType>;
    };
}
