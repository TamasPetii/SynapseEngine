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
#include "MappingExtension.h"
#include "../../../Entity.h"
#include "../Core/SparseVectorMapping.h"
#include <span>

namespace Syn
{
    template<typename DerivedPool>
    struct SparseVectorMappingExtension
    {
    private:
        SYN_INLINE DerivedPool& AsDerived() { return static_cast<DerivedPool&>(*this); }
        SYN_INLINE const DerivedPool& AsDerived() const { return static_cast<const DerivedPool&>(*this); }
    public:
        SYN_INLINE std::span<const DenseIndex> GetSparseIndices() const
        {
            return AsDerived().GetMapping().GetSparseIndices();
        }
    };

    template<>
    struct MappingTraits<SparseVectorMapping>
    {
        template<typename PoolType>
        using Extension = SparseVectorMappingExtension<PoolType>;
    };
}
