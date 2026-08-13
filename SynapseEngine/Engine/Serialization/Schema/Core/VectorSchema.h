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
#include "Engine/Serialization/Schema/Schema.h"
#include <vector>
#include <type_traits>
#include "Engine/Serialization/Archive/Input/IInputArchive.h"

namespace Syn
{
    template <typename T>
    struct Schema<std::vector<T>> {
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