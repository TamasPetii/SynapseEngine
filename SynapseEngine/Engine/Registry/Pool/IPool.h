#pragma once
#include "Engine/Registry/Entity.h"
#include <vector>

namespace Syn
{
    struct IPool
    {
        virtual ~IPool() = default;
        virtual void RemoveIfHas(EntityID entity) = 0;
        virtual void Clear() = 0;
        virtual size_t Size() = 0;
        virtual const std::vector<EntityID>& GetRawEntities() const = 0;
    };
}