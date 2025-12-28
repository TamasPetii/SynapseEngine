#pragma once
#include "StorageBackend.h"
#include "StorageCategory.h"

namespace Syn
{
    template<typename T, bool HasFlags>
    class SegmentedStorageImpl : public StorageBackend<T, HasFlags>
    {
        using Base = StorageBackend<T, HasFlags>;
    public:
        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        U& Get(DenseIndex index);

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        const U& Get(DenseIndex index) const;

        StorageCategory GetCategory(DenseIndex index) const;

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        void Push(EntityID entity, U&& value);

        template<typename U = T, typename = std::enable_if_t<std::is_void_v<U>>>
        void Push(EntityID entity);

        void SetCategory(DenseIndex index, StorageCategory newCat, const SwapCallback& onSwap);
        void Remove(DenseIndex index, const SwapCallback& onSwap);
        void Clear();
    public:
        size_t _staticEnd = 0;
        size_t _dynamicEnd = 0;
    };

    template<typename T> using SegmentedStorage = SegmentedStorageImpl<T, false>;
    template<typename T> using SegmentedStorageFlagged = SegmentedStorageImpl<T, true>;
}

namespace Syn
{
    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE U& SegmentedStorageImpl<T, HasFlags>::Get(DenseIndex index)
    {
        return Base::GetData(index);
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE const U& SegmentedStorageImpl<T, HasFlags>::Get(DenseIndex index) const
    {
        return Base::GetData(index);
    }

    template<typename T, bool HasFlags>
    SYN_INLINE StorageCategory SegmentedStorageImpl<T, HasFlags>::GetCategory(DenseIndex index) const
    {
        if (index < _staticEnd)  return StorageCategory::Static;
        if (index < _dynamicEnd) return StorageCategory::Dynamic;
        return StorageCategory::Stream;
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE void SegmentedStorageImpl<T, HasFlags>::Push(EntityID entity, U&& value)
    {
        Base::PushBackend(entity, std::forward<U>(value));
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE void SegmentedStorageImpl<T, HasFlags>::Push(EntityID entity)
    {
        Base::PushBackend(entity);
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void SegmentedStorageImpl<T, HasFlags>::SetCategory(DenseIndex index, StorageCategory newCat, const SwapCallback& onSwap)
    {
        StorageCategory currentCat = GetCategory(index);

        if (currentCat == newCat) return;

        // 1. Static -> Dynamic (Move Down)
        if (currentCat == StorageCategory::Static && newCat != StorageCategory::Static)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_staticEnd - 1), onSwap);
            _staticEnd--;

            index = static_cast<DenseIndex>(_staticEnd);
            currentCat = StorageCategory::Dynamic;
        }

        // 2. Dynamic -> Stream (Move Down)
        if (currentCat == StorageCategory::Dynamic && newCat == StorageCategory::Stream)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_dynamicEnd - 1), onSwap);
            _dynamicEnd--;
        }

        // 3. Stream -> Dynamic (Move Up)
        if (currentCat == StorageCategory::Stream && newCat != StorageCategory::Stream)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_dynamicEnd), onSwap);
            _dynamicEnd++;

            index = static_cast<DenseIndex>(_dynamicEnd - 1);
            currentCat = StorageCategory::Dynamic;
        }

        // 4. Dynamic -> Static (Move Up)
        if (currentCat == StorageCategory::Dynamic && newCat == StorageCategory::Static)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_staticEnd), onSwap);
            _staticEnd++;
        }
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void SegmentedStorageImpl<T, HasFlags>::Remove(DenseIndex index, const SwapCallback& onSwap)
    {
        const DenseIndex lastIdx = static_cast<DenseIndex>(Base::_entities.size() - 1);

        // If the element is in the Static region, swap it with the last Static element
        if (index < _staticEnd)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_staticEnd - 1), onSwap);
            index = static_cast<DenseIndex>(_staticEnd - 1);
            _staticEnd--;
        }

        // If the element is (now) in the Dynamic region, swap it with the last Dynamic element
        if (index < _dynamicEnd)
        {
            Base::SwapBackend(index, static_cast<DenseIndex>(_dynamicEnd - 1), onSwap);
            index = static_cast<DenseIndex>(_dynamicEnd - 1);
            _dynamicEnd--;
        }

        // At this point, the element is guaranteed to be in the Stream region
        // (or already at the very end of the array).
        // Perform standard removal: swap with last element and pop
        if (index != lastIdx)
        {
            Base::FlagIndexChanged(lastIdx);
            Base::SwapBackend(index, lastIdx, onSwap);
        }

        Base::PopBackend();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void SegmentedStorageImpl<T, HasFlags>::Clear()
    {
        Base::ClearBackend();
        _staticEnd = 0;
        _dynamicEnd = 0;
    }
}