#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

#include <vector>
#include <functional>
#include <type_traits>
#include <span>

#include "../Mixin/Data/DataMixin.h"
#include "../Mixin/Flag/Utils/FlagMixinConcept.h"
#include "../Mixin/Flag/Core/NoFlagMixin.h"

namespace Syn
{
    template<typename T, typename FlagMixinPolicy = NoFlagMixin>
        requires FlagMixinConstraint<FlagMixinPolicy>
    class StorageBackend : public DataMixin<T>, public FlagMixinPolicy
    {
    public:
        template<typename U = T>
            requires (!std::is_void_v<U>)
        SYN_INLINE void PushBackend(EntityID entity, U&& value);

        template<typename U = T>
            requires std::is_void_v<U>
        SYN_INLINE void PushBackend(EntityID entity);

        SYN_INLINE void PopBackend();
        SYN_INLINE void SwapBackend(DenseIndex a, DenseIndex b, const SwapCallback& onSwap);
        SYN_INLINE void ClearBackend();
        SYN_INLINE size_t Size() const;
        SYN_INLINE std::span<const EntityID> GetDenseEntities() const;
    protected:
        std::vector<EntityID> _entities;
    };
}

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE void StorageBackend<T, FlagMixinPolicy>::PushBackend(EntityID entity, U&& value)
    {
        _entities.push_back(entity);
        this->PushData(std::forward<U>(value));
        this->PushFlag();
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    template<typename U>
        requires std::is_void_v<U>
    SYN_INLINE void StorageBackend<T, FlagMixinPolicy>::PushBackend(EntityID entity)
    {
        _entities.push_back(entity);
        this->PushFlag();
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    SYN_INLINE void StorageBackend<T, FlagMixinPolicy>::PopBackend()
    {
        SYN_ASSERT(!_entities.empty(), "Attempting to pop from empty storage");

        _entities.pop_back();
        this->PopData();
        this->PopFlag();
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    SYN_INLINE void StorageBackend<T, FlagMixinPolicy>::SwapBackend(DenseIndex a, DenseIndex b, const SwapCallback& onSwap)
    {
        SYN_ASSERT(a < _entities.size() && b < _entities.size(), "Swap indices out of bounds");

        if (a == b) return;

        std::swap(_entities[a], _entities[b]);
        this->SwapData(a, b);
        this->SwapFlag(a, b);

        if (onSwap) {
            onSwap(_entities[a], a);
            onSwap(_entities[b], b);
        }
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    SYN_INLINE void StorageBackend<T, FlagMixinPolicy>::ClearBackend()
    {
        _entities.clear();
        this->ClearData();
        this->ClearFlags();
        this->ResetAllStateBits();
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    SYN_INLINE size_t StorageBackend<T, FlagMixinPolicy>::Size() const
    {
        return _entities.size();
    }

    template<typename T, typename FlagMixinPolicy>
        requires FlagMixinConstraint<FlagMixinPolicy>
    SYN_INLINE std::span<const EntityID> StorageBackend<T, FlagMixinPolicy>::GetDenseEntities() const
    {
        return _entities;
    }
}