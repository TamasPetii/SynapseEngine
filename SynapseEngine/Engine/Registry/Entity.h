#pragma once
#include <cstdint>
#include <limits>
#include <functional>

namespace Syn {
    using EntityID = uint32_t;
    using DenseIndex = uint32_t;

    constexpr EntityID NULL_ENTITY = std::numeric_limits<EntityID>::max();
    constexpr DenseIndex NULL_INDEX = std::numeric_limits<DenseIndex>::max();

    using SwapCallback = std::function<void(EntityID movedEntity, DenseIndex newIndex)>;
}