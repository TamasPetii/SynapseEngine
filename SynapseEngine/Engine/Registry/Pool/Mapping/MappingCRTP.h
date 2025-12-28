#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"

namespace Syn
{
    /**
     * CRTP Base class to enforce Mapping interface consistency at compile time
     * without virtual function overhead.
     */
    template<typename Derived>
    struct MappingCRTP
    {
        SYN_INLINE void Set(EntityID entity, DenseIndex index)
        {
            static_cast<Derived*>(this)->Set(entity, index);
        }

        SYN_INLINE DenseIndex Get(EntityID entity) const
        {
            return static_cast<const Derived*>(this)->Get(entity);
        }

        SYN_INLINE void Remove(EntityID entity)
        {
            static_cast<Derived*>(this)->Remove(entity);
        }

        SYN_INLINE bool Contains(EntityID entity) const
        {
            return static_cast<const Derived*>(this)->Contains(entity);
        }

        SYN_INLINE void Clear()
        {
            static_cast<Derived*>(this)->Clear();
        }
    };

    template<typename T>
    concept MappingConstraint = std::is_base_of_v<MappingCRTP<T>, T>;
}