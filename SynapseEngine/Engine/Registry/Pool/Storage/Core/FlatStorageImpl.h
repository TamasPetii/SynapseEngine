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
#include "StorageBackend.h"
#include "../Utils/StorageCRTP.h"
#include "../Mixin/Flag/Core/NoFlagMixin.h"
#include "../Mixin/Flag/Core/SimpleFlagMixin.h"

namespace Syn
{
    template<typename T, typename FlagMixinPolicy = NoFlagMixin>
    class FlatStorageImpl :
        public StorageBackend<T, FlagMixinPolicy>,
        public StorageCRTP<FlatStorageImpl<T, FlagMixinPolicy>, T>
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
    };

    template<typename T> using FlatStorage = FlatStorageImpl<T, NoFlagMixin>;
    template<typename T> using FlatStorageFlagged = FlatStorageImpl<T, SimpleFlagMixin>;
    using IndexStorage = FlatStorageImpl<void, NoFlagMixin>;
}

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE U& FlatStorageImpl<T, FlagMixinPolicy>::Get(DenseIndex index)
    {
        return Base::GetData(index);
    }

    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE const U& FlatStorageImpl<T, FlagMixinPolicy>::Get(DenseIndex index) const
    {
        return Base::GetData(index);
    }

    template<typename T, typename FlagMixinPolicy>
    template<typename U>
        requires (!std::is_void_v<U>)
    SYN_INLINE void FlatStorageImpl<T, FlagMixinPolicy>::Push(EntityID entity, U&& value)
    {
        Base::PushBackend(entity, std::forward<U>(value));
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void FlatStorageImpl<T, FlagMixinPolicy>::Push(EntityID entity)
    {
        Base::PushBackend(entity);
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void FlatStorageImpl<T, FlagMixinPolicy>::Remove(DenseIndex index, const SwapCallback& onSwap)
    {
        const DenseIndex lastIdx = static_cast<DenseIndex>(Base::_entities.size() - 1);

        if (index != lastIdx)
        {
            Base::FlagIndexChanged(lastIdx);
            Base::SwapBackend(index, lastIdx, onSwap);
        }

        Base::PopBackend();
    }

    template<typename T, typename FlagMixinPolicy>
    SYN_INLINE void FlatStorageImpl<T, FlagMixinPolicy>::Clear()
    {
        Base::ClearBackend();
    }
}