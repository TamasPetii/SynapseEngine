#pragma once

#include "Pool.h"
#include "Storage/FlatStorageImpl.h"
#include "Storage/SegmentedStorageImpl.h"
#include "Mapping/SparseVectorMapping.h"

namespace Syn
{
    using SparseSet = Pool<void, IndexStorage, SparseVectorMapping>;

    template<typename T>
    using DataPool = Pool<T, FlatStorage<T>, SparseVectorMapping>;

    template<typename T>
    using ComponentPool = Pool<T, SegmentedStorageFlagged<T>, SparseVectorMapping>;
}