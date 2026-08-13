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
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include <vector>
#include <utility>
#include <span>

namespace Syn
{
    class DataMixinInsider;

    template<typename T>
    struct DataMixin
    {
    public:
        SYN_INLINE void PushData(T&& value);
        SYN_INLINE void PushData(const T& value);
        SYN_INLINE void PopData();
        SYN_INLINE void SwapData(DenseIndex a, DenseIndex b);
        SYN_INLINE void ClearData();

        SYN_INLINE T& GetData(DenseIndex index);
        SYN_INLINE const T& GetData(DenseIndex index) const;

        SYN_INLINE std::span<T> GetRawData();
        SYN_INLINE std::span<const T> GetRawData() const;
    protected:
        std::vector<T> _data;
    private:
        friend class DataMixinInsider;
    };

    template<>
    struct DataMixin<void>
    {
        template<typename U>
        SYN_INLINE void PushData(U&&) {}
        SYN_INLINE void PopData() {}
        SYN_INLINE void SwapData(DenseIndex, DenseIndex) {}
        SYN_INLINE void ClearData() {}
    };
}

namespace Syn
{
    template<typename T>
    SYN_INLINE void DataMixin<T>::PushData(T&& value)
    {
        _data.push_back(std::move(value));
    }

    template<typename T>
    SYN_INLINE void DataMixin<T>::PushData(const T& value)
    {
        _data.push_back(value);
    }

    template<typename T>
    SYN_INLINE void DataMixin<T>::PopData()
    {
        SYN_ASSERT(!_data.empty(), "Attempting to pop from empty data vector");
        _data.pop_back();
    }

    template<typename T>
    SYN_INLINE void DataMixin<T>::SwapData(DenseIndex a, DenseIndex b)
    {
        SYN_ASSERT(a < _data.size() && b < _data.size(), "Data index out of bounds");
        std::swap(_data[a], _data[b]);
    }

    template<typename T>
    SYN_INLINE void DataMixin<T>::ClearData()
    {
        _data.clear();
    }

    template<typename T>
    SYN_INLINE T& DataMixin<T>::GetData(DenseIndex index)
    {
        SYN_ASSERT(index < _data.size(), "Data index out of bounds");
        return _data[index];
    }

    template<typename T>
    SYN_INLINE const T& DataMixin<T>::GetData(DenseIndex index) const
    {
        SYN_ASSERT(index < _data.size(), "Data index out of bounds");
        return _data[index];
    }

    template<typename T>
    SYN_INLINE std::span<T> DataMixin<T>::GetRawData()
    {
        return _data;
    }

    template<typename T>
    SYN_INLINE std::span<const T> DataMixin<T>::GetRawData() const
    {
        return _data;
    }
}