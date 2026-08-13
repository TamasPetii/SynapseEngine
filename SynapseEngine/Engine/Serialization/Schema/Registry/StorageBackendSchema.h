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

#include "Engine/Registry/Pool/Storage/Core/StorageBackend.h"
#include "Engine/Registry/Insiders/StorageBackendInsider.h"

namespace Syn
{
    template <typename T>
    concept InitializableFlagConstraint = requires(T mixin, size_t count) {
        mixin.InitializeFlags(count);
    };

    template<typename T, typename FlagMixinPolicy>
    struct Schema<StorageBackend<T, FlagMixinPolicy>> 
    {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);
            auto& entities = StorageBackendInsider::GetEntities(v, StorageBackendInsider::GetKey());

            if (ar.IsBinary())
            {
                BlitVector<EntityID> blitEnts{ entities };
                ar.Property("entities", blitEnts);
            }
            else {
                ar.Property("entities", entities);
            }

            Schema<DataMixin<T>>::Invoke(ar, "components", v);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>)
            {
                if (ar.IsBinary())
                {
                    if constexpr (InitializableFlagConstraint<decltype(v)>)
                    {
                        v.InitializeFlags(entities.size());
                    }
                }
            }
        }
    };
}