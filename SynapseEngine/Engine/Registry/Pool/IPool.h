#pragma once
#include "Engine/Registry/Entity.h"
#include <span>

namespace Syn
{
    struct IPool
    {
        virtual ~IPool() = default;
        virtual void RemoveIfHas(EntityID entity) = 0;
        virtual void Clear() = 0;
        virtual size_t Size() const = 0;
        virtual std::span<const EntityID> GetDenseEntities() const = 0;
        virtual void EnsureEntityMapping(EntityID entity) = 0;
    };
}