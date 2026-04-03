#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Storage/Utils/StorageConcept.h"
#include "Storage/Extension/StorageExtension.h"
#include "Mapping/Utils/MappingConcept.h"
#include "Mapping/Extension/MappingExtension.h"

#include <type_traits>
#include <utility>
#include <span>

namespace Syn
{
    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    class Pool :
        public StorageTraits<StoragePolicy>::template Extension<Pool<T, StoragePolicy, MappingPolicy>>,
        public MappingTraits<MappingPolicy>::template Extension<Pool<T, StoragePolicy, MappingPolicy>>
    {
    public:
        Pool() = default;
        Pool(Pool&&) = default;
        Pool& operator=(Pool&&) = default;
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;

        template<typename U = T> requires (!std::is_void_v<U>)
        SYN_INLINE void Add(EntityID entity, U&& value);

        template<typename U = T> requires (!std::is_void_v<U>&& std::is_default_constructible_v<U>)
        SYN_INLINE void Add(EntityID entity);

        template<typename U = T> requires std::is_void_v<U>
        SYN_INLINE void Add(EntityID entity);

        SYN_INLINE void Remove(EntityID entity);
        SYN_INLINE void Clear();
        SYN_INLINE bool Has(EntityID entity) const;

        template<typename U = T> requires (!std::is_void_v<U>)
        SYN_INLINE U& Get(EntityID entity);

        template<typename U = T> requires (!std::is_void_v<U>)
        SYN_INLINE const U& Get(EntityID entity) const;

        template<typename U = T> requires (!std::is_void_v<U>)
        SYN_INLINE U* TryGet(EntityID entity);

        SYN_INLINE size_t Size() const;

        SYN_INLINE std::span<const EntityID> GetDenseEntities() const;

        template<uint32_t... Bits> 
        SYN_INLINE void SetBit(EntityID entity);

        template<uint32_t... Bits> 
        SYN_INLINE bool IsBitSet(EntityID entity) const;

        template<uint32_t... Bits> 
        SYN_INLINE void ResetBit(EntityID entity);

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSet() const;

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBit();

        SYN_INLINE void ResetAllStateBits();

        SYN_INLINE StoragePolicy& GetStorage() { return _storage; }
        SYN_INLINE const StoragePolicy& GetStorage() const { return _storage; }

        SYN_INLINE MappingPolicy& GetMapping() { return _mapping; }
        SYN_INLINE const MappingPolicy& GetMapping() const { return _mapping; }

        SYN_INLINE uint32_t GetMappingVersion() const { return _mappingVersion; }
        SYN_INLINE void IncrementMappingVersion() { _mappingVersion++; }

        SYN_INLINE uint32_t GetChangeVersion() const { return _changeVersion; }
        SYN_INLINE void IncrementChangeVersion() { _changeVersion++; }

        SYN_INLINE void EnsureEntityMapping(EntityID entity);
    private:
        StoragePolicy _storage;
        MappingPolicy _mapping;
        uint32_t _changeVersion = 1;
        uint32_t _mappingVersion = 1;
    };
}

namespace Syn
{
    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires (!std::is_void_v<U>)
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Add(EntityID entity, U&& value)
    {
        SYN_ASSERT(!_mapping.Contains(entity), "Entity already has this component");
        _storage.Push(entity, std::forward<U>(value));
        _mapping.Set(entity, static_cast<DenseIndex>(_storage.Size() - 1));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires (!std::is_void_v<U>&& std::is_default_constructible_v<U>)
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Add(EntityID entity)
    {
        SYN_ASSERT(!_mapping.Contains(entity), "Entity already has this component");
        _storage.Push(entity, U());
        _mapping.Set(entity, static_cast<DenseIndex>(_storage.Size() - 1));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires std::is_void_v<U>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Add(EntityID entity)
    {
        SYN_ASSERT(!_mapping.Contains(entity), "Entity already has this tag");
        _storage.Push(entity);
        _mapping.Set(entity, static_cast<DenseIndex>(_storage.Size() - 1));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
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
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    SYN_INLINE bool Pool<T, StoragePolicy, MappingPolicy>::Has(EntityID entity) const
    {
        return _mapping.Contains(entity);
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires (!std::is_void_v<U>)
        SYN_INLINE U& Pool<T, StoragePolicy, MappingPolicy>::Get(EntityID entity)
    {
        return _storage.Get(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires (!std::is_void_v<U>)
        SYN_INLINE const U& Pool<T, StoragePolicy, MappingPolicy>::Get(EntityID entity) const
    {
        return _storage.Get(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<typename U> requires (!std::is_void_v<U>)
        SYN_INLINE U* Pool<T, StoragePolicy, MappingPolicy>::TryGet(EntityID entity)
    {
        if (!_mapping.Contains(entity)) return nullptr;
        return &_storage.Get(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    inline std::span<const EntityID> Pool<T, StoragePolicy, MappingPolicy>::GetDenseEntities() const
    {
        return _storage.GetDenseEntities();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::SetBit(EntityID entity)
    {
        _storage.template SetBit<Bits...>(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE bool Pool<T, StoragePolicy, MappingPolicy>::IsBitSet(EntityID entity) const
    {
        return _storage.template IsBitSet<Bits...>(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::ResetBit(EntityID entity)
    {
        _storage.template ResetBit<Bits...>(_mapping.Get(entity));
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::Clear()
    {
        _storage.Clear();
        _mapping.Clear();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    SYN_INLINE size_t Pool<T, StoragePolicy, MappingPolicy>::Size() const
    {
        return _storage.Size();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE bool Pool<T, StoragePolicy, MappingPolicy>::IsStateBitSet() const
    {
        return _storage.template IsStateBitSet<Bits...>();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    template<uint32_t... Bits>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::ResetStateBit()
    {
        _storage.template ResetStateBit<Bits...>();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::ResetAllStateBits()
    {
        _storage.ResetAllStateBits();
    }

    template<typename T, typename StoragePolicy, typename MappingPolicy>
        requires StorageConstraint<StoragePolicy>&& MappingConstraint<MappingPolicy>
    SYN_INLINE void Pool<T, StoragePolicy, MappingPolicy>::EnsureEntityMapping(EntityID entity)
    {
        _mapping.EnsureEntityMapping(entity);
    }
}