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
#include "Engine/Registry/Entity.h"
#include "IPool.h"
#include "PoolConcept.h"
#include <span>

namespace Syn
{
    template<typename PoolType>
        requires PoolConstraint<PoolType>
    class PoolWrapper : public IPool
    {
    public:
        PoolType _pool;

        void RemoveIfHas(EntityID entity) override;
        void Clear() override;
        size_t Size() const override;
        std::span<const EntityID> GetDenseEntities() const override;
        void EnsureEntityMapping(EntityID entity) override;
    };
}

namespace Syn
{
    template<typename PoolType>
        requires PoolConstraint<PoolType>
    void PoolWrapper<PoolType>::RemoveIfHas(EntityID entity)
    {
        if (_pool.Has(entity))
        {
            _pool.Remove(entity);
        }
    }

    template<typename PoolType>
        requires PoolConstraint<PoolType>
    void PoolWrapper<PoolType>::Clear()
    {
        _pool.Clear();
    }

    template<typename PoolType>
        requires PoolConstraint<PoolType>
    size_t PoolWrapper<PoolType>::Size() const
    {
        return _pool.Size();
    }

    template<typename PoolType>
        requires PoolConstraint<PoolType>
    std::span<const EntityID> PoolWrapper<PoolType>::GetDenseEntities() const
    {
        return _pool.GetDenseEntities();
    }

    template<typename PoolType>
        requires PoolConstraint<PoolType>
    void PoolWrapper<PoolType>::EnsureEntityMapping(EntityID entity)
    {
        _pool.EnsureEntityMapping(entity);
    }
}