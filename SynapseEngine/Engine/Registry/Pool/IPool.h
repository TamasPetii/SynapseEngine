#pragma once
#include "Engine/Registry/Entity.h"

namespace Syn
{
    struct IPool
    {
        virtual ~IPool() = default;
        virtual void RemoveIfHas(EntityID entity) = 0;
        virtual void Clear() = 0;
    };
}