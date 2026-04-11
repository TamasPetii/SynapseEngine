#pragma once
#include <vector>

namespace Syn {
    template<typename T>
    struct CpuData {
        std::vector<T> data;

        size_t GetSizeBytes() const { return data.size() * sizeof(T); }
        void Resize(size_t size) { data.resize(size); }
        void AssignZero(size_t size) { data.assign(size, T{}); }
        T* Data() { return data.data(); }
        size_t Size() const { return data.size(); }

        T& operator[](size_t index) {
            return data[index];
        }

        const T& operator[](size_t index) const {
            return data[index];
        }
    };
}