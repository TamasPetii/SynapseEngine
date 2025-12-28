#pragma once
#include "Engine/Registry/Entity.h"
#include "IPool.h"
#include "PoolConcept.h"

namespace Syn
{
    template<typename PoolType>
        requires PoolConstraint<PoolType>
    class PoolWrapper : public IPool
    {
    public:
        PoolType _pool;

        void RemoveIfHas(EntityID entity) override
        {
            if (_pool.Has(entity))
                _pool.Remove(entity);
        }

        void Clear() override
        {
            _pool.Clear();
        }
    };
}