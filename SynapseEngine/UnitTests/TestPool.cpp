#include "gtest/gtest.h"
#include "TestComponents.h"

#include "Engine/Registry/Pool/Pool.h"
#include "Engine/Registry/Pool/Mapping/Extension/SparseVectorMappingExtension.h"
#include "Engine/Registry/Pool/Storage/Extension/SegmentedStorageImplExtension.h"
#include "Engine/Registry/Pool/Mapping/SynMapping.h"
#include "Engine/Registry/Pool/Storage/SynStorage.h"

using namespace Syn;

// Validates the fundamental Create, Read, Update, Delete (CRUD) operations on a standard Flat vector storage. 
// It ensures that entities are added correctly, values can be modified via reference, and that the "swap-and-pop" removal logic maintains data integrity for the remaining entities.
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

// Tests the Sparse Paged storage capability.
// It verifies that the pool can handle widely dispersed Entity IDs (low, mid, and very high) without allocating contiguous memory for all potential IDs, which is the primary benefit of a sparse set.
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

// Verifies the HashMapping backend.
// It ensures that arbitrary, non-sequential Entity IDs can be stored and accessed with O(1) complexity (on average) and that standard removal operations function correctly within the hash map context.
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

// Checks the functionality of the bit-flagged storage wrapper.
// It verifies that specific bits can be set, checked, and reset for individual entities, ensuring that the bitmask is correctly associated with the specific component index.
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

// Crucial test for flagged storage during memory swaps.
// When an entity is removed, the last entity in the dense vector is swapped into its place. This test ensures that the bitflags associated with the "swapped" entity move with it to the new index and do not get overwritten or left behind.
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

// Tests "Tag" components which have no data payload (void type).
// It validates that the pool correctly tracks the existence of these entities without needing to store or retrieve actual component data.
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

// Verifies the Segmented Storage system.
// It checks that entities can be assigned different categories (Static, Dynamic, Stream) and that their data remains accessible and correct regardless of which category segment they currently reside in.
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

// Tests the robustness of moving entities between categories multiple times.
// It ensures that the internal swapping logic holds up when an entity transitions from Static to Stream, then to Dynamic, etc., without losing data or causing corruption.
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

// Verifies removal behavior from the "Static" segment (the beginning of the memory block).
// It checks that when a static entity is removed, the pool correctly fills the gap using other entities while maintaining the boundaries of the Dynamic and Stream segments.
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

// Verifies removal behavior from the "Dynamic" segment (the middle of the memory block).
// It confirms that removing a dynamic entity correctly shifts the remaining dynamic elements (or swaps with the last dynamic one) and preserves the Stream entities that follow.
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

// Performs a stress test by adding a large number of entities and then removing half of them in an alternating pattern.
// This checks the stability of the sparse-dense mapping and the swap-remove logic under heavier load.
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

// Confirms that the `Get` method returns a valid reference that allows modifying the underlying component data directly.
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

// Tests the `Clear` operation.
// Ensures that all entities are properly removed, the size resets to zero, and the pool can be successfully reused after clearing.
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

// Walks an entity through the full promotion chain: Stream -> Dynamic -> Static.
// It verifies that value integrity is maintained throughout and that entering the "Static" category correctly triggers the "Dirty Static" tracking list.
TEST(PoolTest, Segmented_Transition_StreamToStatic) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 10;

    // 1. Init (Stream)
    pool.Add(e, 100);
    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Stream);

    // 2. Stream -> Dynamic
    pool.SetCategory(e, StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Dynamic);
    EXPECT_EQ(pool.Get(e), 100); // Value integrity

    // 3. Dynamic -> Static
    pool.SetCategory(e, StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Static);
    EXPECT_EQ(pool.Get(e), 100);

    // Check Dirty was triggered by entering Static
    auto dirty = pool.GetDirtyStatics();
    EXPECT_EQ(dirty.size(), 1);
    EXPECT_EQ(dirty[0], e);
}

// Tests non-linear category changes (e.g., jumping directly from Stream to Static or vice versa).
// This ensures that the underlying logic can calculate the correct swap path even when skipping the intermediate "Dynamic" category.
TEST(PoolTest, Segmented_Transition_DirectJumps) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e1 = 1;
    EntityID e2 = 2;

    pool.Add(e1, 10);
    pool.Add(e2, 20);

    // Jump Stream -> Static directly
    pool.SetCategory(e1, StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Static);

    // Jump Static -> Stream directly
    pool.SetCategory(e1, StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(e1), StorageCategory::Stream);

    // Verify e2 wasn't affected
    EXPECT_EQ(pool.GetCategory(e2), StorageCategory::Stream);
    EXPECT_EQ(pool.Get(e1), 10);
}

// Creates a mixed layout with entities in all three categories.
// Verifies that the pool maintains the correct categorization for each entity when multiple entities exist in every segment.
TEST(PoolTest, Segmented_MultiElement_Layout) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // Create 2 of each
    EntityID s1 = 1, s2 = 2;
    EntityID d1 = 3, d2 = 4;
    EntityID st1 = 5, st2 = 6;

    // Add all (default Stream)
    pool.Add(s1, 1); pool.Add(s2, 2);
    pool.Add(d1, 3); pool.Add(d2, 4);
    pool.Add(st1, 5); pool.Add(st2, 6);

    // Promote S
    pool.SetCategory(s1, StorageCategory::Static);
    pool.SetCategory(s2, StorageCategory::Static);

    // Promote D
    pool.SetCategory(d1, StorageCategory::Dynamic);
    pool.SetCategory(d2, StorageCategory::Dynamic);

    // Verify Layout
    EXPECT_EQ(pool.GetCategory(s1), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(s2), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(d1), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(d2), StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(st1), StorageCategory::Stream);
    EXPECT_EQ(pool.GetCategory(st2), StorageCategory::Stream);
}

// Tests the "Dirty Static" tracking logic.
// Specifically verifies that:
// 1. Moving an entity into Static marks it dirty.
// 2. Moving an entity OUT of Static causes the entity that swaps into the gap (to fill the void) to ALSO be marked dirty, as its memory address has changed.
TEST(PoolTest, Segmented_DirtyFlags_Logic) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e1 = 1, e2 = 2, e3 = 3;

    pool.Add(e1);
    pool.Add(e2);
    pool.Add(e3);

    // Move e1 to Static -> Should be dirty
    pool.SetCategory(e1, StorageCategory::Static);
    {
        auto dirty = pool.GetDirtyStatics();
        ASSERT_EQ(dirty.size(), 1);
        EXPECT_EQ(dirty[0], e1);
    }

    // Move e2 to Static -> Should be dirty
    pool.SetCategory(e2, StorageCategory::Static);
    {
        auto dirty = pool.GetDirtyStatics();
        ASSERT_EQ(dirty.size(), 1);
        EXPECT_EQ(dirty[0], e2);
    }

    // Move e1 OUT of Static (to Dynamic)
    // When e1 leaves, e2 (which is at the end of static) swaps into e1's place.
    // e2 should now be marked dirty because it moved within memory.
    pool.SetCategory(e1, StorageCategory::Dynamic);
    {
        auto dirty = pool.GetDirtyStatics();
        ASSERT_EQ(dirty.size(), 1);
        EXPECT_EQ(dirty[0], e2); // e2 moved to fill e1's gap
    }

    //FAILED: Need to handle -> Static marked but then changed to dynamic!
}

// Verifies the complex "Cascading Swap" logic when removing an entity from the Static segment.
// Expected behavior:
// 1. The removed Static entity (A) is swapped with the last Static entity (B).
// 2. The Static boundary shrinks. A is now effectively at the end of the Static zone (start of Dynamic).
// 3. A swaps with the last Dynamic entity (C) to maintain the Dynamic block.
// 4. A swaps with the last Stream entity (D) to prepare for pop.
// 5. A is popped.
TEST(PoolTest, Segmented_Remove_From_Static_DeepSwap) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // Layout: [Static: A, B] [Dynamic: C] [Stream: D]
    EntityID A = 1, B = 2, C = 3, D = 4;

    pool.Add(A, 10); pool.SetCategory(A, StorageCategory::Static);
    pool.Add(B, 20); pool.SetCategory(B, StorageCategory::Static);
    pool.Add(C, 30); pool.SetCategory(C, StorageCategory::Dynamic);
    pool.Add(D, 40);

    // Remove A (First Static).
    // Expected Swap Chain:
    // 1. A swaps with B (Last Static). Static Boundary shrinks.
    // 2. A (now at boundary) swaps with C (Last Dynamic). Dynamic Boundary shrinks.
    // 3. A (now at dynamic boundary) swaps with D (Last Stream).
    // 4. Pop.

    pool.Remove(A);

    EXPECT_FALSE(pool.Has(A));
    EXPECT_TRUE(pool.Has(B));
    EXPECT_TRUE(pool.Has(C));
    EXPECT_TRUE(pool.Has(D));

    // B should still be Static
    EXPECT_EQ(pool.GetCategory(B), StorageCategory::Static);
    // C should still be Dynamic
    EXPECT_EQ(pool.GetCategory(C), StorageCategory::Dynamic);
    // D should still be Stream
    EXPECT_EQ(pool.GetCategory(D), StorageCategory::Stream);

    // Values check
    EXPECT_EQ(pool.Get(B), 20);
    EXPECT_EQ(pool.Get(C), 30);
    EXPECT_EQ(pool.Get(D), 40);
}

// Verifies the swap chain when removing from the Dynamic segment.
// Expected behavior:
// 1. The removed Dynamic entity (B) is swapped with the last Dynamic entity (C).
// 2. B (now at Dynamic boundary) swaps with the last Stream entity (D).
// 3. B is popped. Static entities (A) are untouched.
TEST(PoolTest, Segmented_Remove_From_Dynamic_DeepSwap) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // Layout: [Static: A] [Dynamic: B, C] [Stream: D]
    EntityID A = 1, B = 2, C = 3, D = 4;

    pool.Add(A, 1); pool.SetCategory(A, StorageCategory::Static);
    pool.Add(B, 2); pool.SetCategory(B, StorageCategory::Dynamic);
    pool.Add(C, 3); pool.SetCategory(C, StorageCategory::Dynamic);
    pool.Add(D, 4);

    // Remove B (First Dynamic).
    // Expected Swap Chain:
    // 1. B swaps with C (Last Dynamic). Dynamic Boundary shrinks.
    // 2. B (now at boundary) swaps with D (Last Stream).
    // 3. Pop.

    pool.Remove(B);

    EXPECT_FALSE(pool.Has(B));

    EXPECT_EQ(pool.GetCategory(A), StorageCategory::Static);
    EXPECT_EQ(pool.GetCategory(C), StorageCategory::Dynamic); // C took B's spot, stay Dynamic
    EXPECT_EQ(pool.GetCategory(D), StorageCategory::Stream);

    EXPECT_EQ(pool.Get(A), 1);
    EXPECT_EQ(pool.Get(C), 3);
    EXPECT_EQ(pool.Get(D), 4);
}

// Tests removal edge cases.
// 1. Removing the last item in the vector (Stream) should just be a pop.
// 2. Removing the only item (Static) should handle boundary decrements correctly without swapping errors.
TEST(PoolTest, Segmented_Remove_LastItem_NoComplexSwaps) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID A = 1, B = 2;
    pool.Add(A, 10); pool.SetCategory(A, StorageCategory::Static);
    pool.Add(B, 20); // Stream

    // Remove B (Stream, End of vector). Should be simple pop.
    pool.Remove(B);
    EXPECT_FALSE(pool.Has(B));
    EXPECT_TRUE(pool.Has(A));
    EXPECT_EQ(pool.GetCategory(A), StorageCategory::Static);

    // Remove A (Static, only item). 
    // Static boundary decrements, swaps with self/dynamic end (0), pop.
    pool.Remove(A);
    EXPECT_FALSE(pool.Has(A));
    EXPECT_EQ(pool.Size(), 0);
}

// Ensures that clearing the pool resets the internal boundary pointers for Segmented Storage.
// If boundaries aren't reset, new entities added after a Clear() might be incorrectly categorized as Static or Dynamic based on old pointers.
TEST(PoolTest, Segmented_Clear_ResetsBoundaries) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    pool.Add(1, 1); pool.SetCategory(1, StorageCategory::Static);
    pool.Add(2, 2); pool.SetCategory(2, StorageCategory::Dynamic);

    pool.Clear();
    EXPECT_EQ(pool.Size(), 0);

    // Add new entity, should be Stream (index 0)
    pool.Add(3, 3);
    EXPECT_EQ(pool.GetCategory(3), StorageCategory::Stream);

    // If boundaries weren't reset, this might be miscategorized
    pool.SetCategory(3, StorageCategory::Dynamic);
    EXPECT_EQ(pool.GetCategory(3), StorageCategory::Dynamic);
}

// Tests the `INDEX_CHANGED_BIT`.
// When moving from Stream to Dynamic, even if the physical index arguably might not change in an empty Dynamic list scenario, the logic must flag the entity as "Changed" because it crossed a category boundary (implied logical move).
TEST(PoolTest, Segmented_BitFlag_StreamToDynamic_IndexChanged) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 10;

    pool.Add(e, 100);

    // 1. Reset everything to clean slate
    pool.ResetBit<INDEX_CHANGED_BIT>(e);
    pool.ResetBit<DIRTY_STATIC_BIT>(e);

    EXPECT_FALSE((pool.IsBitSet<INDEX_CHANGED_BIT>(e)));

    // 2. Action: Stream -> Dynamic
    // Logic: Swaps with Dynamic boundary. Even if index doesn't physically change (if list is empty), 
    // the system should flag the index as "changed" because it moved categories.
    pool.SetCategory(e, StorageCategory::Dynamic);

    // 3. Verify
    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Dynamic);

    // Index Change Bit should be SET because category changed/swap occured
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(e)));
    // Static Dirty Bit should NOT be set (Dynamic is not Static)
    EXPECT_FALSE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));
}

// Tests flags when entering Static.
// Logic dictates that entering Static MUST set `DIRTY_STATIC_BIT` (to notify systems) and `INDEX_CHANGED_BIT` (because the entity physically moved to the static segment).
TEST(PoolTest, Segmented_BitFlag_DynamicToStatic_BothFlags) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 20;

    pool.Add(e, 200);
    pool.SetCategory(e, StorageCategory::Dynamic);

    // 1. Reset
    pool.ResetBit<INDEX_CHANGED_BIT>(e);
    pool.ResetBit<DIRTY_STATIC_BIT>(e);

    // 2. Action: Dynamic -> Static
    // This moves the entity into the Static segment.
    pool.SetCategory(e, StorageCategory::Static);

    // 3. Verify
    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Static);

    // Should be flagged as moved/changed
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(e)));
    // MUST be marked Dirty Static because it entered the static region
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));

    // Check dirty list content
    auto dirty = pool.GetDirtyStatics();
    ASSERT_EQ(dirty.size(), 1);
    EXPECT_EQ(dirty[0], e);
}

// A critical integrity test for the "Gap Filler".
// Scenario: Entity A (Static) moves to Dynamic.
// Result: Entity A is no longer Static (loses dirty flag).
// Crucially: Entity B (the last Static entity) swaps into A's place. B is still Static, but its index changed. Therefore, B MUST be marked `DIRTY_STATIC` so renderers know its offset changed.
TEST(PoolTest, Segmented_BitFlag_StaticToDynamic_GapFill_Integrity) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // A and B are Static.
    EntityID A = 1, B = 2;
    pool.Add(A); pool.SetCategory(A, StorageCategory::Static);
    pool.Add(B); pool.SetCategory(B, StorageCategory::Static);

    // 1. Reset all flags
    pool.ResetBit<INDEX_CHANGED_BIT>(A); pool.ResetBit<DIRTY_STATIC_BIT>(A);
    pool.ResetBit<INDEX_CHANGED_BIT>(B); pool.ResetBit<DIRTY_STATIC_BIT>(B);
    pool.ResetStaticDirtyCounter();

    // 2. Action: Move A (Static) -> Dynamic
    // Logic:
    // - A is removed from Static.
    // - B (last static) swaps into A's place to fill the gap.
    // - A moves to Dynamic.
    pool.SetCategory(A, StorageCategory::Dynamic);

    // 3. Verify Entity A (The Mover)
    EXPECT_EQ(pool.GetCategory(A), StorageCategory::Dynamic);
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(A))); // A moved indices
    EXPECT_FALSE((pool.IsBitSet<DIRTY_STATIC_BIT>(A))); // A is no longer Static

    // 4. Verify Entity B (The Gap Filler)
    EXPECT_EQ(pool.GetCategory(B), StorageCategory::Static);
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(B))); // B moved to fill A's spot

    // CRITICAL: B stayed in Static but moved index, so it MUST be marked Dirty Static
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(B)));

    // Dirty List check
    auto dirty = pool.GetDirtyStatics();
    ASSERT_EQ(dirty.size(), 1);
    EXPECT_EQ(dirty[0], B); // Only B is in static dirty list now
}

// Verifies flag correctness during a Cascading Swap caused by removing a Static entity.
// Chain Logic:
// 1. Remove S1. S2 (End Static) swaps with S1 -> S2 moves -> S2 Flagged.
// 2. S1 (now at boundary) swaps with D1 (End Dynamic) -> D1 moves -> D1 Flagged.
// 3. S1 (now at boundary) swaps with ST1 (End Stream) -> ST1 moves -> ST1 Flagged.
// Result: All entities moved to fill gaps must be marked as Changed. S2 must specifically be Dirty Static.
TEST(PoolTest, Segmented_BitFlag_RemoveStatic_CascadingSwaps) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // Layout: [Static: S1, S2] [Dynamic: D1] [Stream: ST1]
    EntityID S1 = 1, S2 = 2, D1 = 3, ST1 = 4;

    pool.Add(S1); pool.SetCategory(S1, StorageCategory::Static);
    pool.Add(S2); pool.SetCategory(S2, StorageCategory::Static);
    pool.Add(D1); pool.SetCategory(D1, StorageCategory::Dynamic);
    pool.Add(ST1);

    // Reset everything
    pool.ResetBit<INDEX_CHANGED_BIT>(S1); pool.ResetBit<DIRTY_STATIC_BIT>(S1);
    pool.ResetBit<INDEX_CHANGED_BIT>(S2); pool.ResetBit<DIRTY_STATIC_BIT>(S2);
    pool.ResetBit<INDEX_CHANGED_BIT>(D1);
    pool.ResetBit<INDEX_CHANGED_BIT>(ST1);
    pool.ResetStaticDirtyCounter();

    // Action: Remove S1 (First Static)
    // Chain expectation:
    // 1. S2 (End Static) swaps with S1. S2 moves. -> S2 Flagged.
    // 2. S1 (now at Static boundary) swaps with D1 (End Dynamic). D1 moves. -> D1 Flagged.
    // 3. S1 (now at Dynamic boundary) swaps with ST1 (End Stream). ST1 moves. -> ST1 Flagged.
    // 4. S1 popped.
    pool.Remove(S1);

    EXPECT_FALSE(pool.Has(S1));

    // Verify S2 (Gap filler for Static)
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(S2)));
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(S2))); // Moved within static

    // Verify D1 (Gap filler for the boundary shift)
    // D1 was at the end of Dynamic, swapped to make room for the shrinking Static segment?
    // Actually, implementation: 
    //  - Swap(S1, S2). S2 is now at 0.
    //  - S1 is at _staticEnd.
    //  - _staticEnd decrements. S1 is technically "in Dynamic" zone now temporarily.
    //  - If S1 < _dynamicEnd (yes), Swap(S1, D1). D1 moves to where S1 was.
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(D1)));
    // D1 is Dynamic, so usually no Dirty Static bit (unless logic forces it, but shouldn't)
    EXPECT_FALSE((pool.IsBitSet<DIRTY_STATIC_BIT>(D1)));

    // Verify ST1 (Gap filler for final pop)
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(ST1)));
}

// Tests flag integrity when an entity round-trips: Static -> Stream -> Static.
// Verifies that flags are correctly set when leaving Static (should lose dirty status) and correctly re-set when re-entering Static (should regain dirty status).
TEST(PoolTest, Segmented_BitFlag_Complex_CategorySwap_And_Back) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 5;

    pool.Add(e);
    // Start at Static
    pool.SetCategory(e, StorageCategory::Static);

    // Clear flags
    pool.ResetBit<INDEX_CHANGED_BIT>(e);
    pool.ResetBit<DIRTY_STATIC_BIT>(e);
	pool.ResetStaticDirtyCounter();

    // 1. Move Static -> Stream (Direct Jump)
    // Implementation usually does: Static -> Dynamic -> Stream logic internally
    pool.SetCategory(e, StorageCategory::Stream);

    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Stream);
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(e)));
    // It left static, so it shouldn't be dirty static anymore
    EXPECT_FALSE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));

    // Clear flags again
    pool.ResetBit<INDEX_CHANGED_BIT>(e);

    // 2. Move Stream -> Static (Direct Jump)
    pool.SetCategory(e, StorageCategory::Static);

    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Static);
    EXPECT_TRUE((pool.IsBitSet<INDEX_CHANGED_BIT>(e)));
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(e))); // Entered static
}

// Tests the idempotency of `MarkStaticDirty`.
// Verifies that marking an entity dirty multiple times does not result in duplicate entries in the "Dirty Statics" list.
TEST(PoolTest, Segmented_BitFlag_StaticDirty_Idempotency) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 10;

    pool.Add(e);
    pool.SetCategory(e, StorageCategory::Static);
    pool.ResetBit<DIRTY_STATIC_BIT>(e); // Manually clear
    pool.ResetStaticDirtyCounter();


    // 1. Mark Dirty Explicitly
    pool.MarkStaticDirty(e);
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));

    // 2. Reset the Index Changed bit to ensure MarkStaticDirty doesn't touch it unnecessarily
    // (Though usually MarkStaticDirty only touches the dirty bit)
    pool.ResetBit<INDEX_CHANGED_BIT>(e);

    pool.MarkStaticDirty(e); // Mark again

    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));

    // Verify we didn't duplicate in the list
    auto dirty = pool.GetDirtyStatics();
    int count = 0;
    for (auto id : dirty) {
        if (id == e) count++;
    }
    EXPECT_EQ(count, 1);
}

// Tests the "Zombie" dirty entry scenario. 
// If a Static entity is marked dirty but moves to Dynamic before the frame ends, the dirty list might still contain the ID (because O(N) removal from the list is too slow). 
// However, the entity's internal dirty bitflag MUST be cleared so that systems iterating the dirty list can check the flag and safely ignore this "zombie" entry.
TEST(PoolTest, Segmented_Zombie_Dirty_Cleanup) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 10;

    pool.Add(e);

    // 1. Move to Static -> Marks as Dirty
    pool.SetCategory(e, StorageCategory::Static);

    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Static);
    EXPECT_TRUE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));

    // Verify it is technically in the list
    {
        auto dirty = pool.GetDirtyStatics();
        bool found = false;
        for (auto id : dirty) if (id == e) found = true;
        EXPECT_TRUE(found);
    }

    // 2. The "OOPS" moment: Move it to Dynamic immediately
    pool.SetCategory(e, StorageCategory::Dynamic);

    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Dynamic);

    // CRITICAL: The bitflag on the entity must be cleared, even if the ID lingers in the vector.
    EXPECT_FALSE((pool.IsBitSet<DIRTY_STATIC_BIT>(e)));
}

// Verifies idempotency when setting a category.
// Setting an entity to the category it already occupies (e.g., Static -> Static) should be a strict no-op. 
// It must not trigger swaps, memory moves, or falsely raise dirty flags.
TEST(PoolTest, EdgeCase_Segmented_Idempotency_SelfSet) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 10;

    pool.Add(e, 50);
    pool.SetCategory(e, StorageCategory::Static);

    // ACTION: Set Static -> Static (No-Op expected)
    pool.SetCategory(e, StorageCategory::Static);

    EXPECT_EQ(pool.GetCategory(e), StorageCategory::Static);
    EXPECT_EQ(pool.Get(e), 50);

    // Ensure no false dirty flags were raised
    auto dirty = pool.GetDirtyStatics();
    EXPECT_EQ(dirty.size(), 1);
}

// Tests a "Domino" deletion pattern where entities are removed sequentially from the start of the memory block (Static segment).
// This forces the Static and Dynamic boundaries to collapse repeatedly, ensuring the complex swap-and-pop logic handles continuous boundary shifting without corruption.
TEST(PoolTest, EdgeCase_Domino_Deletion_From_Start) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    // Layout: 3 Static, 3 Dynamic, 3 Stream
    std::vector<EntityID> entities;
    for (int i = 0; i < 9; ++i) {
        entities.push_back(i);
        pool.Add(i, i);
    }

    pool.SetCategory(0, StorageCategory::Static);
    pool.SetCategory(1, StorageCategory::Static);
    pool.SetCategory(2, StorageCategory::Static);

    pool.SetCategory(3, StorageCategory::Dynamic);
    pool.SetCategory(4, StorageCategory::Dynamic);
    pool.SetCategory(5, StorageCategory::Dynamic);

    EXPECT_EQ(pool.Size(), 9);

    // Remove items by ID 0..8. 
    // Since these were added sequentially, removing 'i' effectively targets the start/boundaries.
    for (int i = 0; i < 9; ++i) {
        pool.Remove(i);
        EXPECT_EQ(pool.Size(), 9 - (i + 1));
    }

    EXPECT_EQ(pool.Size(), 0);
}

// Checks for "Ghost Data" on ID reuse.
// When an entity is removed and its ID is later reused for a new entity, the new instance must be completely clean.
// It verifies that old bit-flags (metadata) do not persist across the lifecycle of the ID.
TEST(PoolTest, EdgeCase_Ghost_Data_Reuse) {
    Pool<int, FlatStorageFlagged<int>, SparseVectorMapping> pool;
    EntityID e = 666;

    // 1. Setup "Old" Entity with dirty state
    pool.Add(e, 100);
    pool.SetBit<5>(e);

    // 2. Kill it
    pool.Remove(e);
    EXPECT_FALSE(pool.Has(e));

    // 3. Reincarnate: Add SAME ID back
    pool.Add(e, 999);

    // 4. Verify "New" Entity is clean
    EXPECT_TRUE(pool.Has(e));
    EXPECT_EQ(pool.Get(e), 999);
    EXPECT_FALSE(pool.IsBitSet<5>(e));
}

// Performs a "Ping-Pong" stress test.
// An entity is rapidly swapped back and forth between Static and Dynamic categories.
// This ensures internal boundary pointers (`_staticEnd`, `_dynamicEnd`) do not drift and that other entities swapped in the process remain valid.
TEST(PoolTest, EdgeCase_Boundary_Thrashing) {
    Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;
    EntityID e = 1;
    EntityID other = 2;

    pool.Add(e, 10);
    pool.Add(other, 20);

    pool.SetCategory(e, StorageCategory::Stream);
    pool.SetCategory(other, StorageCategory::Stream);

    // Thrash e between Static and Dynamic
    for (int i = 0; i < 100; ++i) {
        pool.SetCategory(e, StorageCategory::Static);
        ASSERT_EQ(pool.GetCategory(e), StorageCategory::Static);

        pool.SetCategory(e, StorageCategory::Dynamic);
        ASSERT_EQ(pool.GetCategory(e), StorageCategory::Dynamic);
    }

    EXPECT_EQ(pool.Size(), 2);
    EXPECT_EQ(pool.Get(e), 10);

    // Check if 'other' (the bystander) is still valid after 200 swaps
    EXPECT_EQ(pool.Get(other), 20);
}