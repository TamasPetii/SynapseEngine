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
#include "Engine/Serialization/Schema/Core/VectorSchema.h" 

#include "Engine/Registry/Pool/Storage/Mixin/Data/DataMixin.h"
#include "Engine/Registry/Insiders/DataMixinInsider.h"

namespace Syn
{
    template<typename T>
    struct Schema<DataMixin<T>> {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            if constexpr (!std::is_void_v<T>) 
            {
                auto& v = const_cast<std::remove_const_t<U>&>(val);
                auto& data = DataMixinInsider::GetData(v, DataMixinInsider::GetKey());

                if constexpr (!std::is_trivially_copyable_v<T>) {
                    ar.Property("data", data);
                }
                else {
                    if (ar.IsBinary())
                    {
                        BlitVector<T> blitData{ data };
                        ar.Property("data", blitData);
                    }
                    else
                    {
                        ar.Property("data", data);
                    }
                }
            }
        }
    };
}