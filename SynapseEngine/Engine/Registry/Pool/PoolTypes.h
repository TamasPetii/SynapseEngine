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

#include "Pool.h"
#include "Storage/Core/FlatStorageImpl.h"
#include "Storage/Core/SegmentedStorageImpl.h"
#include "Storage/Extension/SegmentedStorageImplExtension.h"
#include "Mapping/Core/SparseVectorMapping.h"
#include "Mapping/Extension/SparseVectorMappingExtension.h"

namespace Syn
{
    using SparseSet = Pool<void, IndexStorage, SparseVectorMapping>;

    template<typename T>
    using DataPool = Pool<T, FlatStorage<T>, SparseVectorMapping>;

    template<typename T>
    using ComponentPool = Pool<T, SegmentedStorage<T>, SparseVectorMapping>;
}