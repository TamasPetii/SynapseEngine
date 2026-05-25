#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include <vector>
#include <type_traits>
#include "Engine/Serialization/Archive/Input/IInputArchive.h"

namespace Syn
{
    template <typename T>
    struct SYN_API Schema<std::vector<T>> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) {
            ScopedArchiveObject obj(ar, name);

            uint32_t arraySize = static_cast<uint32_t>(val.size());

            ar.EnterArray("data", arraySize);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>) {
                val.resize(arraySize);
            }

            for (uint32_t i = 0; i < arraySize; ++i) {
                ar.Property("item", val[i]);
            }

            ar.LeaveArray();
        }
    };

    template <typename T>
    struct BlitVector {
        std::vector<T>& vec;
    };

    template <typename T>
    struct Schema<BlitVector<T>> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) {
            static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for raw memory serialization!");

            ScopedArchiveObject obj(ar, name);

            uint32_t count = static_cast<uint32_t>(val.vec.size());
            ar.Property("count", count);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>)
                val.vec.resize(count);
            
            if (count > 0)
                ar.PropertyBytes("data", val.vec.data(), count * sizeof(T));
        }
    };
}