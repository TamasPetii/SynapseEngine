#include "gtest/gtest.h"
#include "TestComponents.h"

#include "Engine/Registry/Pool/Pool.h"
#include "Engine/Registry/Pool/Mapping/SynMapping.h"
#include "Engine/Registry/Pool/Storage/SynStorage.h"

using namespace Syn;

TEST(PoolTest, FlatVector_BasicCRUD) {
    Pool<Vec3, FlatStorage<Vec3>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, { 1.0f, 1.0f, 1.0f });
    pool.Add(e2, { 2.0f, 2.0f, 2.0f });
    pool.Add(e3, { 3.0f, 3.0f, 3.0f });

    EXPECT_EQ(pool.Size(), 3);
    EXPECT_TRUE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e2));
    EXPECT_TRUE(pool.Has(e3));

    Vec3& val = pool.Get(e2);
    EXPECT_FLOAT_EQ(val.y, 2.0f);

    val.y = 20.0f;
    EXPECT_FLOAT_EQ(pool.Get(e2).y, 20.0f);

    pool.Remove(e2);
    EXPECT_EQ(pool.Size(), 2);
    EXPECT_FALSE(pool.Has(e2));
    EXPECT_TRUE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e3));

    EXPECT_FLOAT_EQ(pool.Get(e1).x, 1.0f);
    EXPECT_FLOAT_EQ(pool.Get(e3).x, 3.0f);

    pool.Remove(e1);
    pool.Remove(e3);
    EXPECT_EQ(pool.Size(), 0);
}

TEST(PoolTest, FlatPaged_HighIDs) {
    Pool<int, FlatStorage<int>, SparsePagedMapping> pool;

    EntityID low = 5;
    EntityID mid = 5000;
    EntityID high = 200000;

    pool.Add(low, 5);
    pool.Add(mid, 5000);
    pool.Add(high, 200000);

    EXPECT_EQ(pool.Size(), 3);
    EXPECT_EQ(pool.Get(low), 5);
    EXPECT_EQ(pool.Get(mid), 5000);
    EXPECT_EQ(pool.Get(high), 200000);

    pool.Remove(mid);
    EXPECT_FALSE(pool.Has(mid));
    EXPECT_TRUE(pool.Has(high));
    EXPECT_TRUE(pool.Has(low));

    EXPECT_EQ(pool.Get(high), 200000);
}

TEST(PoolTest, FlatHash_RandomAccess) {
    Pool<float, FlatStorage<float>, HashMapping> pool;

    EntityID e1 = 99999999;
    EntityID e2 = 12345;

    pool.Add(e1, 1.5f);
    pool.Add(e2, 2.5f);

    EXPECT_EQ(pool.Size(), 2);
    EXPECT_FLOAT_EQ(pool.Get(e1), 1.5f);

    pool.Remove(e1);
    EXPECT_FALSE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e2));
    EXPECT_FLOAT_EQ(pool.Get(e2), 2.5f);
}

TEST(PoolTest, FlaggedVector_Bits) {
    Pool<int, FlatStorageFlagged<int>, SparseVectorMapping> pool;

    EntityID e1 = 10;
    EntityID e2 = 20;

    pool.Add(e1, 100);
    pool.Add(e2, 200);

    pool.SetBit<5>(e1);
    pool.SetBit<6>(e1);

    EXPECT_TRUE(pool.IsBitSet<5>(e1));
    EXPECT_TRUE(pool.IsBitSet<6>(e1));
    EXPECT_FALSE(pool.IsBitSet<5>(e2));

    pool.ResetBit<5>(e1);
    EXPECT_FALSE(pool.IsBitSet<5>(e1));
    EXPECT_TRUE(pool.IsBitSet<6>(e1));
}

TEST(PoolTest, FlaggedVector_SwapIntegrity) {
    Pool<int, FlatStorageFlagged<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, 10);
    pool.Add(e2, 20);
    pool.Add(e3, 30);

    pool.SetBit<5>(e1);
    pool.SetBit<5>(e3);

    EXPECT_TRUE(pool.IsBitSet<5>(e1));
    EXPECT_FALSE(pool.IsBitSet<5>(e2));
    EXPECT_TRUE(pool.IsBitSet<5>(e3));

    pool.Remove(e2);

    EXPECT_EQ(pool.Size(), 2);
    EXPECT_TRUE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e3));

    EXPECT_EQ(pool.Get(e1), 10);
    EXPECT_EQ(pool.Get(e3), 30);

    EXPECT_TRUE(pool.IsBitSet<5>(e1));
    EXPECT_TRUE(pool.IsBitSet<5>(e3));

    pool.Remove(e1);

    EXPECT_TRUE(pool.Has(e3));
    EXPECT_EQ(pool.Get(e3), 30);
    EXPECT_TRUE(pool.IsBitSet<5>(e3));
}

TEST(PoolTest, TagVector) {
    Pool<void, IndexStorage, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;

    pool.Add(e1);
    pool.Add(e2);

    EXPECT_TRUE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e2));
    EXPECT_EQ(pool.Size(), 2);

    pool.Remove(e1);
    EXPECT_FALSE(pool.Has(e1));
    EXPECT_TRUE(pool.Has(e2));
}

TEST(PoolTest, Segmented_Categories) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, 100);
    pool.Add(e2, 200);
    pool.Add(e3, 300);

    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(e2), StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(e3), StorageCategory::Stream);

    pool.SetCategory(e1, StorageCategory::Static);
    pool.SetCategory(e2, StorageCategory::Dynamic);

    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(e2), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(e3), StorageCategory::Stream);

    EXPECT_EQ(pool.Get(e1), 100);
    EXPECT_EQ(pool.Get(e2), 200);
    EXPECT_EQ(pool.Get(e3), 300);
}

TEST(PoolTest, Segmented_ComplexMoves) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;

    pool.Add(e1, 10);
    pool.Add(e2, 20);

    pool.SetCategory(e1, StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Static);

    pool.SetCategory(e1, StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Stream);
    EXPECT_EQ(pool.Get(e1), 10);

    pool.SetCategory(e1, StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Dynamic);

    pool.SetCategory(e2, StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(e2), StorageCategory::Static);
}

TEST(PoolTest, Segmented_Remove_Static_GapFill) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID s1 = 1, s2 = 2;
    EntityID d1 = 3, d2 = 4;
    EntityID st1 = 5, st2 = 6;

    pool.Add(s1, 1); pool.SetCategory(s1, StorageCategory::Static);
    pool.Add(s2, 2); pool.SetCategory(s2, StorageCategory::Static);

    pool.Add(d1, 3); pool.SetCategory(d1, StorageCategory::Dynamic);
    pool.Add(d2, 4); pool.SetCategory(d2, StorageCategory::Dynamic);

    pool.Add(st1, 5);
    pool.Add(st2, 6);

    EXPECT_EQ(pool.Size(), 6);

    pool.Remove(s1);

    EXPECT_EQ(pool.Size(), 5);
    EXPECT_FALSE(pool.Has(s1));

    EXPECT_EQ(pool.GetCategory(s2), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(d1), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(d2), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(st1), StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(st2), StorageCategory::Stream);

    EXPECT_EQ(pool.Get(s2), 2);
    EXPECT_EQ(pool.Get(d1), 3);
    EXPECT_EQ(pool.Get(d2), 4);
    EXPECT_EQ(pool.Get(st1), 5);
    EXPECT_EQ(pool.Get(st2), 6);
}

TEST(PoolTest, Segmented_Remove_Dynamic_GapFill) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID s1 = 1;
    EntityID d1 = 2, d2 = 3;
    EntityID st1 = 4;

    pool.Add(s1, 1); pool.SetCategory(s1, StorageCategory::Static);
    pool.Add(d1, 2); pool.SetCategory(d1, StorageCategory::Dynamic);
    pool.Add(d2, 3); pool.SetCategory(d2, StorageCategory::Dynamic);
    pool.Add(st1, 4);

    pool.Remove(d1);

    EXPECT_FALSE(pool.Has(d1));
    EXPECT_EQ(pool.GetCategory(s1), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(d2), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(st1), StorageCategory::Stream);

    EXPECT_EQ(pool.Get(d2), 3);
}

TEST(PoolTest, StressTest_LargeScale) {
    Pool<int, FlatStorage<int>, SparseVectorMapping> pool;

    constexpr int COUNT = 10000;

    for (int i = 0; i < COUNT; ++i) {
        pool.Add(i, i * 2);
    }
    EXPECT_EQ(pool.Size(), COUNT);

    for (int i = 0; i < COUNT; i += 2) {
        pool.Remove(i);
    }
    EXPECT_EQ(pool.Size(), COUNT / 2);

    for (int i = 0; i < COUNT; i += 2) {
        EXPECT_FALSE(pool.Has(i));
    }
    for (int i = 1; i < COUNT; i += 2) {
        EXPECT_TRUE(pool.Has(i));
        EXPECT_EQ(pool.Get(i), i * 2);
    }
}

TEST(PoolTest, UpdateExisting) {
    Pool<Vec3, FlatStorage<Vec3>, SparseVectorMapping> pool;
    EntityID e = 1;

    pool.Add(e, { 0, 0, 0 });

    Vec3& ref = pool.Get(e);
    ref.x = 5.0f;

    EXPECT_FLOAT_EQ(pool.Get(e).x, 5.0f);

    pool.Get(e) = { 10.0f, 10.0f, 10.0f };
    EXPECT_FLOAT_EQ(pool.Get(e).y, 10.0f);
}

TEST(PoolTest, Clear) {
    Pool<int, FlatStorage<int>, SparseVectorMapping> pool;

    pool.Add(1, 10);
    pool.Add(2, 20);
    pool.Add(3, 30);

    pool.Clear();

    EXPECT_EQ(pool.Size(), 0);
    EXPECT_FALSE(pool.Has(1));
    EXPECT_FALSE(pool.Has(2));
    EXPECT_FALSE(pool.Has(3));

    pool.Add(1, 99);
    EXPECT_EQ(pool.Size(), 1);
    EXPECT_EQ(pool.Get(1), 99);
}