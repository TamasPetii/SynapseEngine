#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Storage/StorageCategory.h"
#include "Mapping/MappingCRTP.h"
#include <type_traits>
#include <utility>

namespace Syn
{
    template<typename T, typename StoragePolicy, typename MappingPolicy>
       requires MappingConstraint<MappingPolicy>
    class Pool
    {
    public:
        Pool() = default;
        Pool(Pool&&) = default;
        Pool& operator=(Pool&&) = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        void Add(EntityID entity, U&& value);

        template<typename U = T, typename = std::enable_if_t<std::is_void_v<U>>>
        void Add(EntityID entity);

        void Remove(EntityID entity);
        void Clear();

        bool      Has(EntityID entity) const;

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        U& Get(EntityID entity);

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        const U& Get(EntityID entity) const;

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        U* TryGet(EntityID entity);

        size_t    Size() const;

        void            SetCategory(EntityID entity, StorageCategory newCat);
        StorageCategory GetCategory(EntityID entity) const;

        template<uint32_t... Bits>
        void SetBit(EntityID entity);

        template<uint32_t... Bits>
        bool IsBitSet(EntityID entity) const;

        template<uint32_t... Bits>
        void ResetBit(EntityID entity);
    public:
        StoragePolicy _storage;
        MappingPolicy _mapping;
    };
}

namespace Syn
{
    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<typename U, typename>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Add(EntityID entity, U&& value)
    {
        SYN_ASSERT(!_mapping.Contains(entity), "Entity already has this component");

        _storage.Push(entity, std::forward<U>(value));

        const DenseIndex index = static_cast<DenseIndex>(_storage.Size() - 1);

        _mapping.Set(entity, index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<typename U, typename>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Add(EntityID entity)
    {
        SYN_ASSERT(!_mapping.Contains(entity), "Entity already has this tag");

        _storage.Push(entity);

        const DenseIndex index = static_cast<DenseIndex>(_storage.Size() - 1);

        _mapping.Set(entity, index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Remove(EntityID entity)
    {
        SYN_ASSERT(_mapping.Contains(entity), "Entity does not have this component");

        const DenseIndex index = _mapping.Get(entity);

        _storage.Remove(index, [this](EntityID movedEntity, DenseIndex newIndex) {
            _mapping.Set(movedEntity, newIndex);
            });

        _mapping.Remove(entity);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE bool Pool<T, StoragePolicy, MappingPolicy>::Has(EntityID entity) const
    {
        return _mapping.Contains(entity);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<typename U, typename>
    SYN_INLINE U& Pool<T, StoragePolicy, MappingPolicy>::Get(EntityID entity)
    {
        const DenseIndex index = _mapping.Get(entity);
        return _storage.Get(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<typename U, typename>
    SYN_INLINE const U& Pool<T, StoragePolicy, MappingPolicy>::Get(EntityID entity) const
    {
        const DenseIndex index = _mapping.Get(entity);
        return _storage.Get(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<typename U, typename>
    SYN_INLINE U* Pool<T, StoragePolicy, MappingPolicy>::TryGet(EntityID entity)
    {
        if (!_mapping.Contains(entity))
            return nullptr;

        return &_storage.Get(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::SetCategory(EntityID entity, StorageCategory newCat)
    {
        SYN_ASSERT(_mapping.Contains(entity), "Entity not in pool");

        const DenseIndex index = _mapping.Get(entity);

        _storage.SetCategory(index, newCat, [this](EntityID movedEntity, DenseIndex newIndex) {
            _mapping.Set(movedEntity, newIndex);
            });
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE StorageCategory Pool<T, StoragePolicy, MappingPolicy>::GetCategory(EntityID entity) const
    {
        const DenseIndex index = _mapping.Get(entity);
        return _storage.GetCategory(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::SetBit(EntityID entity)
    {
        const DenseIndex index = _mapping.Get(entity);
        _storage.template SetBit<Bits...>(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE bool Pool<T, StoragePolicy, MappingPolicy>::IsBitSet(EntityID entity) const
    {
        const DenseIndex index = _mapping.Get(entity);
        return _storage.template IsBitSet<Bits...>(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::ResetBit(EntityID entity)
    {
        const DenseIndex index = _mapping.Get(entity);
        _storage.template ResetBit<Bits...>(index);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Clear()
    {
        _storage.Clear();
        _mapping.Clear();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires MappingConstraint<MappingPolicy>
    SYN_INLINE size_t Pool<T, StoragePolicy, MappingPolicy>::Size() const
    {
        return _storage.Size();
    }
}