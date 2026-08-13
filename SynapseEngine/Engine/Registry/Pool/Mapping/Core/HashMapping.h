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

#include "../../../Entity.h"
#include "../Utils/MappingCRTP.h"

#include <unordered_map>

namespace Syn
{
    class SYN_API HashMapping : public MappingCRTP<HashMapping>
    {
    public:
        SYN_INLINE void Set(EntityID entity, DenseIndex index);
        SYN_INLINE DenseIndex Get(EntityID entity) const;
        SYN_INLINE void Remove(EntityID entity);
        SYN_INLINE bool Contains(EntityID entity) const;
        SYN_INLINE void Clear();
        SYN_INLINE void EnsureEntityMapping(EntityID entity) {} //Todo?
    private:
        std::unordered_map<EntityID, DenseIndex> _map;
    };
}

namespace Syn
{
    SYN_INLINE void HashMapping::Set(EntityID entity, DenseIndex index)
    {
        _map[entity] = index;
    }

    SYN_INLINE DenseIndex HashMapping::Get(EntityID entity) const
    {
        SYN_ASSERT(_map.contains(entity), "Entity not found in HashMapping");
        return _map.at(entity);
    }

    SYN_INLINE void HashMapping::Remove(EntityID entity)
    {
        SYN_ASSERT(_map.contains(entity), "Attempting to remove non-existent entity from HashMapping");
        _map.erase(entity);
    }

    SYN_INLINE bool HashMapping::Contains(EntityID entity) const
    {
        return _map.find(entity) != _map.end();
    }

    SYN_INLINE void HashMapping::Clear()
    {
        _map.clear();
    }
}