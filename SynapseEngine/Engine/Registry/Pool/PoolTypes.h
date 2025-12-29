#pragma once

#include "Pool.h"
#include "Storage/Core/FlatStorageImpl.h"
#include "Storage/Core/SegmentedStorageImpl.h"
#include "Mapping/Core/SparseVectorMapping.h"

namespace Syn
{
    using SparseSet = Pool<void, IndexStorage, SparseVectorMapping>;

    template<typename T>
    using DataPool = Pool<T, FlatStorage<T>, SparseVectorMapping>;

    template<typename T>
    using ComponentPool = Pool<T, SegmentedStorage<T>, SparseVectorMapping>;
}