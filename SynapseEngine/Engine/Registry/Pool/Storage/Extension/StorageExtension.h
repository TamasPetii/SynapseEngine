#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

namespace Syn
{
    template<typename PoolType>
    struct DefaultStorageExtension {};

    template<typename StoragePolicy>
    struct StorageTraits
    {
        template<typename PoolType>
        using Extension = DefaultStorageExtension<PoolType>;
    };
}
