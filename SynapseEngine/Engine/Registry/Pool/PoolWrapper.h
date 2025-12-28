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
}