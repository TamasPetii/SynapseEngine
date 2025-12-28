#include <GLFW/glfw3.h>
#include "Engine/Logger/SynLog.h"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

#include "Engine/Registry/Pool/Pool.h"
#include "Engine/Registry/Pool/Mapping/SynMapping.h"
#include "Engine/Registry/Pool/Storage/SynStorage.h"

using namespace Syn;

#define TEST_CASE(name) \
    std::cout << "\n[TEST] " << name << "\n"; \
    try {

#define TEST_END \
        std::cout << "   [PASS]\n"; \
    } catch (const std::exception& e) { \
        std::cout << "   [FAIL] " << e.what() << "\n"; \
        exit(1); \
    }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "   Assertion Failed: " << #a << " != " << #b << "\n"; \
        std::cerr << "   Expected: " << (b) << ", Actual: " << (a) << "\n"; \
        exit(1); \
    }

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        std::cerr << "   Assertion Failed: " << #cond << " is false\n"; \
        exit(1); \
    }

#define ASSERT_FALSE(cond) \
    if ((cond)) { \
        std::cerr << "   Assertion Failed: " << #cond << " is true\n"; \
        exit(1); \
    }

struct Vec3 {
    float x, y, z;
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct Matrix4 {
    float data[16];
};

void Test_Flat_Vector_BasicCRUD()
{
    TEST_CASE("FlatStorage + VectorMapping: Basic CRUD")
        Pool<Vec3, FlatStorage<Vec3>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, { 1.0f, 1.0f, 1.0f });
    pool.Add(e2, { 2.0f, 2.0f, 2.0f });
    pool.Add(e3, { 3.0f, 3.0f, 3.0f });

    ASSERT_EQ(pool.Size(), 3);
    ASSERT_TRUE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e2));
    ASSERT_TRUE(pool.Has(e3));

    Vec3& val = pool.Get(e2);
    ASSERT_EQ(val.y, 2.0f);
    val.y = 20.0f;
    ASSERT_EQ(pool.Get(e2).y, 20.0f);

    pool.Remove(e2);
    ASSERT_EQ(pool.Size(), 2);
    ASSERT_FALSE(pool.Has(e2));
    ASSERT_TRUE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e3));

    ASSERT_EQ(pool.Get(e1).x, 1.0f);
    ASSERT_EQ(pool.Get(e3).x, 3.0f);

    pool.Remove(e1);
    pool.Remove(e3);
    ASSERT_EQ(pool.Size(), 0);

    TEST_END
}

void Test_Flat_Paged_HighIDs()
{
    TEST_CASE("FlatStorage + PagedMapping: High ID Handling")
        Pool<int, FlatStorage<int>, SparsePagedMapping> pool;

    EntityID low = 5;
    EntityID mid = 5000;
    EntityID high = 200000;

    pool.Add(low, 5);
    pool.Add(mid, 5000);
    pool.Add(high, 200000);

    ASSERT_EQ(pool.Size(), 3);
    ASSERT_EQ(pool.Get(low), 5);
    ASSERT_EQ(pool.Get(mid), 5000);
    ASSERT_EQ(pool.Get(high), 200000);

    pool.Remove(mid);
    ASSERT_FALSE(pool.Has(mid));
    ASSERT_TRUE(pool.Has(high));
    ASSERT_TRUE(pool.Has(low));

    ASSERT_EQ(pool.Get(high), 200000);

    TEST_END
}

void Test_Flat_Hash_RandomAccess()
{
    TEST_CASE("FlatStorage + HashMapping: Random/Sparse Access")
        Pool<float, FlatStorage<float>, HashMapping> pool;

    EntityID e1 = 99999999;
    EntityID e2 = 12345;

    pool.Add(e1, 1.5f);
    pool.Add(e2, 2.5f);

    ASSERT_EQ(pool.Size(), 2);
    ASSERT_EQ(pool.Get(e1), 1.5f);

    pool.Remove(e1);
    ASSERT_FALSE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e2));
    ASSERT_EQ(pool.Get(e2), 2.5f);

    TEST_END
}

void Test_Flagged_Vector_Bits()
{
    TEST_CASE("FlatStorageFlagged + VectorMapping: Bit Operations")
        Pool<int, FlatStorageFlagged<int>, SparseVectorMapping> pool;

    EntityID e1 = 10;
    EntityID e2 = 20;

    pool.Add(e1, 100);
    pool.Add(e2, 200);

    pool.SetBit<5>(e1);
    pool.SetBit<6>(e1);

    ASSERT_TRUE(pool.IsBitSet<5>(e1));
    ASSERT_TRUE(pool.IsBitSet<6>(e1));
    ASSERT_FALSE(pool.IsBitSet<5>(e2));

    pool.ResetBit<5>(e1);
    ASSERT_FALSE(pool.IsBitSet<5>(e1));
    ASSERT_TRUE(pool.IsBitSet<6>(e1));

    TEST_END
}

void Test_Flagged_Vector_SwapIntegrity()
{
    TEST_CASE("FlatStorageFlagged: Data/Flag Integrity on Remove")
        Pool<int, FlatStorageFlagged<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, 10);
    pool.Add(e2, 20);
    pool.Add(e3, 30);

    pool.SetBit<5>(e1);
    pool.SetBit<5>(e3);

    ASSERT_TRUE(pool.IsBitSet<5>(e1));
    ASSERT_FALSE(pool.IsBitSet<5>(e2));
    ASSERT_TRUE(pool.IsBitSet<5>(e3));

    pool.Remove(e2);

    ASSERT_EQ(pool.Size(), 2);
    ASSERT_TRUE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e3));

    ASSERT_EQ(pool.Get(e1), 10);
    ASSERT_EQ(pool.Get(e3), 30);

    ASSERT_TRUE(pool.IsBitSet<5>(e1));
    ASSERT_TRUE(pool.IsBitSet<5>(e3));

    pool.Remove(e1);

    ASSERT_TRUE(pool.Has(e3));
    ASSERT_EQ(pool.Get(e3), 30);
    ASSERT_TRUE(pool.IsBitSet<5>(e3));

    TEST_END
}

void Test_Tag_Vector()
{
    TEST_CASE("IndexStorage (Tag) + VectorMapping")
        Pool<void, IndexStorage, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;

    pool.Add(e1);
    pool.Add(e2);

    ASSERT_TRUE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e2));
    ASSERT_EQ(pool.Size(), 2);

    pool.Remove(e1);
    ASSERT_FALSE(pool.Has(e1));
    ASSERT_TRUE(pool.Has(e2));

    TEST_END
}

void Test_Segmented_Categories()
{
    TEST_CASE("SegmentedStorage: Category Assignment")
        Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;
    EntityID e3 = 3;

    pool.Add(e1, 100);
    pool.Add(e2, 200);
    pool.Add(e3, 300);

    ASSERT_EQ(pool.GetCategory(e1), StorageCategory::Stream);
    ASSERT_EQ(pool.GetCategory(e2), StorageCategory::Stream);
    ASSERT_EQ(pool.GetCategory(e3), StorageCategory::Stream);

    pool.SetCategory(e1, StorageCategory::Static);
    pool.SetCategory(e2, StorageCategory::Dynamic);

    ASSERT_EQ(pool.GetCategory(e1), StorageCategory::Static);
    ASSERT_EQ(pool.GetCategory(e2), StorageCategory::Dynamic);
    ASSERT_EQ(pool.GetCategory(e3), StorageCategory::Stream);

    ASSERT_EQ(pool.Get(e1), 100);
    ASSERT_EQ(pool.Get(e2), 200);
    ASSERT_EQ(pool.Get(e3), 300);

    TEST_END
}

void Test_Segmented_ComplexMoves()
{
    TEST_CASE("SegmentedStorage: Complex Category Transitions")
        Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID e1 = 1;
    EntityID e2 = 2;

    pool.Add(e1, 10);
    pool.Add(e2, 20);

    pool.SetCategory(e1, StorageCategory::Static);
    ASSERT_EQ(pool.GetCategory(e1), StorageCategory::Static);

    pool.SetCategory(e1, StorageCategory::Stream);
    ASSERT_EQ(pool.GetCategory(e1), StorageCategory::Stream);
    ASSERT_EQ(pool.Get(e1), 10);

    pool.SetCategory(e1, StorageCategory::Dynamic);
    ASSERT_EQ(pool.GetCategory(e1), StorageCategory::Dynamic);

    pool.SetCategory(e2, StorageCategory::Static);
    ASSERT_EQ(pool.GetCategory(e2), StorageCategory::Static);

    TEST_END
}

void Test_Segmented_Remove_Static_GapFill()
{
    TEST_CASE("SegmentedStorage: Remove from Static (Boundary Check)")
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

    ASSERT_EQ(pool.Size(), 6);

    pool.Remove(s1);

    ASSERT_EQ(pool.Size(), 5);
    ASSERT_FALSE(pool.Has(s1));

    ASSERT_EQ(pool.GetCategory(s2), StorageCategory::Static);
    ASSERT_EQ(pool.GetCategory(d1), StorageCategory::Dynamic);
    ASSERT_EQ(pool.GetCategory(d2), StorageCategory::Dynamic);
    ASSERT_EQ(pool.GetCategory(st1), StorageCategory::Stream);
    ASSERT_EQ(pool.GetCategory(st2), StorageCategory::Stream);

    ASSERT_EQ(pool.Get(s2), 2);
    ASSERT_EQ(pool.Get(d1), 3);
    ASSERT_EQ(pool.Get(d2), 4);
    ASSERT_EQ(pool.Get(st1), 5);
    ASSERT_EQ(pool.Get(st2), 6);

    TEST_END
}

void Test_Segmented_Remove_Dynamic_GapFill()
{
    TEST_CASE("SegmentedStorage: Remove from Dynamic")
        Pool<int, SegmentedStorage<int>, SparseVectorMapping> pool;

    EntityID s1 = 1;
    EntityID d1 = 2, d2 = 3;
    EntityID st1 = 4;

    pool.Add(s1, 1); pool.SetCategory(s1, StorageCategory::Static);
    pool.Add(d1, 2); pool.SetCategory(d1, StorageCategory::Dynamic);
    pool.Add(d2, 3); pool.SetCategory(d2, StorageCategory::Dynamic);
    pool.Add(st1, 4);

    pool.Remove(d1);

    ASSERT_FALSE(pool.Has(d1));
    ASSERT_EQ(pool.GetCategory(s1), StorageCategory::Static);
    ASSERT_EQ(pool.GetCategory(d2), StorageCategory::Dynamic);
    ASSERT_EQ(pool.GetCategory(st1), StorageCategory::Stream);

    ASSERT_EQ(pool.Get(d2), 3);

    TEST_END
}

void Test_LargeScale_Operations()
{
    TEST_CASE("Stress Test: Add/Remove Loop")
        Pool<int, FlatStorage<int>, SparseVectorMapping> pool;

    constexpr int COUNT = 10000;

    for (int i = 0; i < COUNT; ++i) {
        pool.Add(i, i * 2);
    }
    ASSERT_EQ(pool.Size(), COUNT);

    for (int i = 0; i < COUNT; i += 2) {
        pool.Remove(i);
    }
    ASSERT_EQ(pool.Size(), COUNT / 2);

    for (int i = 0; i < COUNT; i += 2) {
        ASSERT_FALSE(pool.Has(i));
    }
    for (int i = 1; i < COUNT; i += 2) {
        ASSERT_TRUE(pool.Has(i));
        ASSERT_EQ(pool.Get(i), i * 2);
    }

    TEST_END
}

void Test_Update_Existing()
{
    TEST_CASE("Update Existing Component")
        Pool<Vec3, FlatStorage<Vec3>, SparseVectorMapping> pool;
    EntityID e = 1;

    pool.Add(e, { 0, 0, 0 });

    Vec3& ref = pool.Get(e);
    ref.x = 5.0f;

    ASSERT_EQ(pool.Get(e).x, 5.0f);

    pool.Get(e) = { 10.0f, 10.0f, 10.0f };
    ASSERT_EQ(pool.Get(e).y, 10.0f);

    TEST_END
}

void Test_Pool_Clear()
{
    TEST_CASE("Pool Clear")
        Pool<int, FlatStorage<int>, SparseVectorMapping> pool;

    pool.Add(1, 10);
    pool.Add(2, 20);
    pool.Add(3, 30);

    pool.Clear();

    ASSERT_EQ(pool.Size(), 0);
    ASSERT_FALSE(pool.Has(1));
    ASSERT_FALSE(pool.Has(2));
    ASSERT_FALSE(pool.Has(3));

    pool.Add(1, 99);
    ASSERT_EQ(pool.Size(), 1);
    ASSERT_EQ(pool.Get(1), 99);

    TEST_END
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "   STARTING ECS POOL COMPREHENSIVE TESTS\n";
    std::cout << "========================================\n";

    Test_Flat_Vector_BasicCRUD();
    Test_Flat_Paged_HighIDs();
    Test_Flat_Hash_RandomAccess();
    Test_Flagged_Vector_Bits();
    Test_Flagged_Vector_SwapIntegrity();
    Test_Tag_Vector();
    Test_Segmented_Categories();
    Test_Segmented_ComplexMoves();
    Test_Segmented_Remove_Static_GapFill();
    Test_Segmented_Remove_Dynamic_GapFill();
    Test_Update_Existing();
    Test_Pool_Clear();
    Test_LargeScale_Operations();

    std::cout << "========================================\n";
    std::cout << "   ALL TESTS PASSED\n";
    std::cout << "========================================\n";

    Syn::Logger::Get().AddSink(std::make_shared<Syn::ConsoleSink>());
    Syn::Logger::Get().AddSink(std::make_shared<Syn::MemorySink>());
    Syn::Logger::Get().AddSink(std::make_shared<Syn::FileSink>());

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(100, 100, "Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}