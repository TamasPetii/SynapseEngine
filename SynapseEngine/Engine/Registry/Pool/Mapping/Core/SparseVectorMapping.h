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

#include <vector>
#include <span>

namespace Syn
{
    class SparseMappingInsider;

    class SYN_API SparseVectorMapping : public MappingCRTP<SparseVectorMapping>
    {
    public:
        SYN_INLINE void Set(EntityID entity, DenseIndex index);
        SYN_INLINE DenseIndex Get(EntityID entity) const;
        SYN_INLINE void Remove(EntityID entity);
        SYN_INLINE bool Contains(EntityID entity) const;
        SYN_INLINE void Clear();
        SYN_INLINE void EnsureEntityMapping(EntityID entity);
    public:
        std::span<const DenseIndex> GetSparseIndices() const;
    private:
        std::vector<DenseIndex> _indices;
    private:
        friend class SparseMappingInsider;
    };
}

namespace Syn
{
    SYN_INLINE void SparseVectorMapping::Set(EntityID entity, DenseIndex index)
    {
        if (entity >= _indices.size())
        {
            _indices.resize(entity + 1, NULL_INDEX);
        }

        //SYN_ASSERT(_indices[entity] == NULL_INDEX, "Entity already mapped in VectorMapping");

        _indices[entity] = index;
    }

    SYN_INLINE DenseIndex SparseVectorMapping::Get(EntityID entity) const
    {
        SYN_ASSERT(entity < _indices.size(), "Entity ID out of bounds");
        SYN_ASSERT(_indices[entity] != NULL_INDEX, "Entity ID not mapped");
        return _indices[entity];
    }

    SYN_INLINE void SparseVectorMapping::Remove(EntityID entity)
    {
        SYN_ASSERT(entity < _indices.size(), "Entity ID out of bounds during remove");
        _indices[entity] = NULL_INDEX;
    }

    SYN_INLINE bool SparseVectorMapping::Contains(EntityID entity) const
    {
        return entity < _indices.size() && _indices[entity] != NULL_INDEX;
    }

    SYN_INLINE void SparseVectorMapping::Clear()
    {
        _indices.clear();
    }

    SYN_INLINE std::span<const DenseIndex> SparseVectorMapping::GetSparseIndices() const
    {
        return _indices;
    }

    SYN_INLINE void SparseVectorMapping::EnsureEntityMapping(EntityID entity)
    {
        if (entity >= _indices.size())
        {
            _indices.resize(entity + 1, NULL_INDEX);
        }
    }
}