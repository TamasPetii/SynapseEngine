#pragma once
#include "StorageBackend.h"

namespace Syn
{
    template<typename T, bool HasFlags>
    class FlatStorageImpl : public StorageBackend<T, HasFlags>
    {
        using Base = StorageBackend<T, HasFlags>;
    public:
        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        U& Get(DenseIndex index);

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        const U& Get(DenseIndex index) const;

        template<typename U = T, typename = std::enable_if_t<!std::is_void_v<U>>>
        void Push(EntityID entity, U&& value);

        template<typename U = T, typename = std::enable_if_t<std::is_void_v<U>>>
        void Push(EntityID entity);

        void Remove(DenseIndex index, const SwapCallback& onSwap);
        void Clear();
    };

    template<typename T> using FlatStorage = FlatStorageImpl<T, false>;
    template<typename T> using FlatStorageFlagged = FlatStorageImpl<T, true>;
    using IndexStorage = FlatStorageImpl<void, false>;
}

namespace Syn
{
    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE U& FlatStorageImpl<T, HasFlags>::Get(DenseIndex index)
    {
        return Base::GetData(index);
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE const U& FlatStorageImpl<T, HasFlags>::Get(DenseIndex index) const
    {
        return Base::GetData(index);
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE void FlatStorageImpl<T, HasFlags>::Push(EntityID entity, U&& value)
    {
        Base::PushBackend(entity, std::forward<U>(value));
    }

    template<typename T, bool HasFlags>
    template<typename U, typename>
    SYN_INLINE void FlatStorageImpl<T, HasFlags>::Push(EntityID entity)
    {
        Base::PushBackend(entity);
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void FlatStorageImpl<T, HasFlags>::Remove(DenseIndex index, const SwapCallback& onSwap)
    {
        const DenseIndex lastIdx = static_cast<DenseIndex>(Base::_entities.size() - 1);

        if (index != lastIdx)
        {
            Base::FlagIndexChanged(lastIdx);
            Base::SwapBackend(index, lastIdx, onSwap);
        }

        Base::PopBackend();
    }

    template<typename T, bool HasFlags>
    SYN_INLINE void FlatStorageImpl<T, HasFlags>::Clear()
    {
        Base::ClearBackend();
    }
}