#pragma once
#include "StorageBackend.h"

#include "../Utils/StorageCRTP.h"
#include "../Utils/StorageCategory.h"
#include "../Mixin/Flag/Core/SimpleFlagMixin.h"

#include <vector>
#include <atomic>
#include <span>

namespace Syn
{
    template<typename T, typename FlagMixinPolicy = SimpleFlagMixin>
    class SegmentedStorageImpl :
        public StorageBackend<T, FlagMixinPolicy>,
        public StorageCRTP<SegmentedStorageImpl<T, FlagMixinPolicy>, T>
    {
        using Base = StorageBackend<T, FlagMixinPolicy>;
    public:
        using ValueType = T;
        using Base::Size;

        template<typename U = T>
            requires (!std::is_void_v<U>)
        SYN_INLINE U& Get(DenseIndex index);

        template<typename U = T>
            requires (!std::is_void_v<U>)
        SYN_INLINE const U& Get(DenseIndex index) const;

        template<typename U = T>
            requires (!std::is_void_v<U>)
        SYN_INLINE void Push(EntityID entity, U&& value);

        SYN_INLINE void Push(EntityID entity);

        SYN_INLINE void Remove(DenseIndex index, const SwapCallback& onSwap);

        SYN_INLINE void Clear();

        SYN_INLINE StorageCategory GetCategory(DenseIndex index) const;

        SYN_INLINE void SetCategory(DenseIndex index, StorageCategory newCat, const SwapCallback& onSwap);

        SYN_INLINE void MarkStaticDirty(DenseIndex index);

        SYN_INLINE std::span<EntityID> GetDirtyStatics();

        SYN_INLINE void ResetStaticDirtyCounter();
    private:
        SYN_INLINE void EnsureDirtyCapacity();
    protected:
        size_t _staticEnd = 0;
        size_t _dynamicEnd = 0;
        std::vector<EntityID> _dirtyStaticList;
        std::atomic<size_t>   _dirtyStaticCount{ 0 };
    };

    template<typename T> using SegmentedStorage = SegmentedStorageImpl<T, SimpleFlagMixin>;
}

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE U& SegmentedStorageImpl<T, FlagMixinPolicy>::Get(DenseIndex index)
    {
        return Base::GetData(index);
    }

    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE const U& SegmentedStorageImpl<T, FlagMixinPolicy>::Get(DenseIndex index) const
    {
        return Base::GetData(index);
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE StorageCategory SegmentedStorageImpl<T, FlagMixinPolicy>::GetCategory(DenseIndex index) const
    {
        if (index < _staticEnd)  return StorageCategory::Static;
        if (index < _dynamicEnd) return StorageCategory::Dynamic;
        return StorageCategory::Stream;
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::EnsureDirtyCapacity()
    {
        [[unlikely]]
        if (_dirtyStaticList.size() < Base::Size())
            _dirtyStaticList.resize(Base::Size());
    }

    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::Push(EntityID entity, U&& value)
    {
        Base::PushBackend(entity, std::forward<U>(value));
        EnsureDirtyCapacity();
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::Push(EntityID entity)
    {
        Base::PushBackend(entity);
        EnsureDirtyCapacity();
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::SetCategory(DenseIndex index, StorageCategory newCat, const SwapCallback& onSwap)
    {
        StorageCategory currentCat = GetCategory(index);

        if (currentCat == newCat) return;

        if (currentCat == StorageCategory::Static && newCat != StorageCategory::Static)
        {
            Base::template ResetBit<DIRTY_STATIC_BIT>(index);

            const DenseIndex swapTarget = static_cast<DenseIndex>(_staticEnd - 1);

            Base::FlagIndexChanged(index);
            Base::FlagIndexChanged(swapTarget);

            Base::SwapBackend(index, swapTarget, onSwap);
            _staticEnd--;

            MarkStaticDirty(index);

            index = static_cast<DenseIndex>(_staticEnd);
            currentCat = StorageCategory::Dynamic;
        }

        if (currentCat == StorageCategory::Dynamic && newCat == StorageCategory::Stream)
        {
            const DenseIndex swapTarget = static_cast<DenseIndex>(_dynamicEnd - 1);

            Base::FlagIndexChanged(index);
            Base::FlagIndexChanged(swapTarget);

            Base::SwapBackend(index, swapTarget, onSwap);
            _dynamicEnd--;
        }

        if (currentCat == StorageCategory::Stream && newCat != StorageCategory::Stream)
        {
            const DenseIndex swapTarget = static_cast<DenseIndex>(_dynamicEnd);

            Base::FlagIndexChanged(index);
            Base::FlagIndexChanged(swapTarget);

            Base::SwapBackend(index, swapTarget, onSwap);
            _dynamicEnd++;

            index = static_cast<DenseIndex>(_dynamicEnd - 1);
            currentCat = StorageCategory::Dynamic;
        }

        if (currentCat == StorageCategory::Dynamic && newCat == StorageCategory::Static)
        {
            const DenseIndex swapTarget = static_cast<DenseIndex>(_staticEnd);

            Base::FlagIndexChanged(index);
            Base::FlagIndexChanged(swapTarget);

            Base::SwapBackend(index, swapTarget, onSwap);
            _staticEnd++;

            MarkStaticDirty(static_cast<DenseIndex>(_staticEnd - 1));
        }
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::Remove(DenseIndex index, const SwapCallback& onSwap)
    {
        const DenseIndex lastIdx = static_cast<DenseIndex>(Base::_entities.size() - 1);

        if (index < _staticEnd)
        {
            const DenseIndex swapTarget = static_cast<DenseIndex>(_staticEnd - 1);

            Base::FlagIndexChanged(swapTarget);
            Base::SwapBackend(index, swapTarget, onSwap);

            MarkStaticDirty(index);

            index = swapTarget;
            _staticEnd--;
        }

        if (index < _dynamicEnd)
        {
            const DenseIndex swapTarget = static_cast<DenseIndex>(_dynamicEnd - 1);

            Base::FlagIndexChanged(swapTarget);
            Base::SwapBackend(index, swapTarget, onSwap);

            index = swapTarget;
            _dynamicEnd--;
        }

        if (index != lastIdx)
        {
            Base::FlagIndexChanged(lastIdx);
            Base::SwapBackend(index, lastIdx, onSwap);
        }

        Base::PopBackend();
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::Clear()
    {
        Base::ClearBackend();
        _staticEnd = 0;
        _dynamicEnd = 0;
        _dirtyStaticCount.store(0, std::memory_order_relaxed);
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::MarkStaticDirty(DenseIndex index)
    {
        if (index >= _staticEnd)
            return;

        if (Base::template SetBit<DIRTY_STATIC_BIT>(index))
            return;

        size_t dirtyIdx = _dirtyStaticCount.fetch_add(1, std::memory_order_relaxed);

        SYN_ASSERT(dirtyIdx < _dirtyStaticList.size(), "Dirty Static List overflow! Deduplication or Resize failed.");

        _dirtyStaticList[dirtyIdx] = Base::_entities[index];
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE std::span<EntityID> SegmentedStorageImpl<T, FlagMixinPolicy>::GetDirtyStatics()
    {
        size_t count = _dirtyStaticCount.load(std::memory_order_acquire);

        SYN_ASSERT(count <= _dirtyStaticList.size(), "Dirty count is larger than buffer size!");

        return { _dirtyStaticList.data(), count };
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void SegmentedStorageImpl<T, FlagMixinPolicy>::ResetStaticDirtyCounter()
    {
        _dirtyStaticCount.store(0, std::memory_order_release);
    }
}