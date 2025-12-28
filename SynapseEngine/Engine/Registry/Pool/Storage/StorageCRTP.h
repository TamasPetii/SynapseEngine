#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include <type_traits>
#include <utility>

namespace Syn
{
    template<typename Derived, typename DataType>
    struct StorageCRTP
    {
        template<typename U = DataType>
            requires (!std::is_void_v<U>)
        SYN_INLINE U& Get(DenseIndex index)
        {
            return static_cast<Derived*>(this)->template Get<U>(index);
        }

        template<typename U = DataType>
            requires (!std::is_void_v<U>)
        SYN_INLINE const U& Get(DenseIndex index) const
        {
            return static_cast<const Derived*>(this)->template Get<U>(index);
        }

        template<typename U = DataType>
            requires (!std::is_void_v<U>)
        SYN_INLINE void Push(EntityID entity, U&& value)
        {
            static_cast<Derived*>(this)->Push(entity, std::forward<U>(value));
        }

        SYN_INLINE void Push(EntityID entity)
        {
            static_cast<Derived*>(this)->Push(entity);
        }

        SYN_INLINE void Remove(DenseIndex index, const SwapCallback& onSwap)
        {
            static_cast<Derived*>(this)->Remove(index, onSwap);
        }

        SYN_INLINE void Clear()
        {
            static_cast<Derived*>(this)->Clear();
        }

        SYN_INLINE size_t Size() const
        {
            return static_cast<const Derived*>(this)->Size();
        }
    };
}