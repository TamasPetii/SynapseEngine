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
#include "Engine/Mesh/Data/Common/MeshInstanceDescriptor.h"

namespace Syn
{
    template <>
    struct Schema<MeshInstanceDescriptor> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& d = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", d.name);
            ar.Property("meshIndex", d.meshIndex);
            ar.Property("nodeIndex", d.nodeIndex);
            ar.Property("parentNodeIndex", d.parentNodeIndex);
            ar.Property("vertexCount", d.vertexCount);
            ar.Property("indexCount", d.indexCount);
        }
    };
}