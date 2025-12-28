#pragma once
#include <type_traits>
#include "Pool.h"

namespace Syn
{
    template<typename T>
    struct IsSynPool : std::false_type {};

    template<typename T, typename Storage, typename Mapping>
    struct IsSynPool<Pool<T, Storage, Mapping>> : std::true_type {};

    template<typename T>
    constexpr bool IsSynPool_v = IsSynPool<T>::value;

    template<typename T>
    concept PoolConstraint = IsSynPool_v<T>;
}