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

#include "Engine/Registry/Pool/Storage/Core/SegmentedStorageImpl.h"
#include "Engine/Registry/Insiders/SegmentedStorageInsider.h"

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
    struct Schema<SegmentedStorageImpl<T, FlagMixinPolicy>> 
    {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);

            auto& sEnd = SegmentedStorageInsider::GetStaticEnd(v, SegmentedStorageInsider::GetKey());
            auto& dEnd = SegmentedStorageInsider::GetDynamicEnd(v, SegmentedStorageInsider::GetKey());

            uint32_t staticEnd = static_cast<uint32_t>(sEnd);
            uint32_t dynamicEnd = static_cast<uint32_t>(dEnd);

            ar.Property("staticEnd", staticEnd);
            ar.Property("dynamicEnd", dynamicEnd);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>) {
                sEnd = staticEnd;
                dEnd = dynamicEnd;
            }

            Schema<StorageBackend<T, FlagMixinPolicy>>::Invoke(ar, "backend", v);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>)
            {
                if (ar.IsBinary())
                {
                    auto& dirtyStaticList = SegmentedStorageInsider::GetDirtyStaticList(v, SegmentedStorageInsider::GetKey());
                    dirtyStaticList.resize(v.Size());
                }
            }
        }
    };
}