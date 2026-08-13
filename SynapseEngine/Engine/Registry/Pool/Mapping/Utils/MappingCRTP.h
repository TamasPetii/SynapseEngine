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

namespace Syn
{
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

        SYN_INLINE void EnsureEntityMapping(EntityID entity)
        {
            static_cast<Derived*>(this)->EnsureEntityMapping(entity);
        }
    };
}