#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "../Mixin/DataMixin.h"
#include "../Mixin/FlagMixin.h"
#include <vector>
#include <functional>
#include <type_traits>
#include <span>

namespace Syn
{
    template<typename T, bool HasFlags>
    class StorageBackend : public DataMixin<T>, public FlagMixin<HasFlags>
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
    template<typename T, bool HasFlags>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE void StorageBackend<T, HasFlags>::PushBackend(EntityID entity, U&& value)
    {
        _entities.push_back(entity);
        this->PushData(std::forward<U>(value));
        this->PushFlag();
    }

    template<typename T, bool HasFlags>
    template<typename U>
        requires std::is_void_v<U>
    SYN_INLINE void StorageBackend<T, HasFlags>::PushBackend(EntityID entity)
    {
        _entities.push_back(entity);
        this->PushFlag();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void StorageBackend<T, HasFlags>::PopBackend()
    {
        SYN_ASSERT(!_entities.empty(), "Attempting to pop from empty storage");

        _entities.pop_back();
        this->PopData();
        this->PopFlag();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void StorageBackend<T, HasFlags>::SwapBackend(DenseIndex a, DenseIndex b, const SwapCallback& onSwap)
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

    template<typename T, bool HasFlags>
    SYN_INLINE void StorageBackend<T, HasFlags>::ClearBackend()
    {
        _entities.clear();
        this->ClearData();
        this->ClearFlags();
        this->ResetAllStateBits();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE size_t StorageBackend<T, HasFlags>::Size() const
    {
        return _entities.size();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE std::span<const EntityID> StorageBackend<T, HasFlags>::GetDenseEntities() const
    {
        return _entities;
    }
}