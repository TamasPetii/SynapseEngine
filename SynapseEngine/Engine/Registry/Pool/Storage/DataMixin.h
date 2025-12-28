#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include <vector>
#include <utility>

namespace Syn
{
    template<typename T>
    struct DataMixin
    {
    public:
        void PushData(T&& value);
        void PushData(const T& value);
        void PopData();
        void SwapData(DenseIndex a, DenseIndex b);
        void ClearData();

        T& GetData(DenseIndex index);
        const T& GetData(DenseIndex index) const;
    protected:
        std::vector<T> _data;
    };

    template<>
    struct DataMixin<void>
    {
        template<typename U>
        SYN_INLINE void PushData(U&&) {}

        SYN_INLINE void PopData() {}
        SYN_INLINE void SwapData(DenseIndex, DenseIndex) {}
        SYN_INLINE void ClearData() {}
        SYN_INLINE const void* GetDataPtr() const { return nullptr; }
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
}