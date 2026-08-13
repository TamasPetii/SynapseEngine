// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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