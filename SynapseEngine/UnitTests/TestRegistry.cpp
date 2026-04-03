#include "gtest/gtest.h"
#include "TestComponents.h"
#include "Engine/Registry/Registry.h"

using namespace Syn;

TEST(RegistryTest, EntityLifecycle) {
    Registry reg;

    EntityID e1 = reg.CreateEntity();
    EntityID e2 = reg.CreateEntity();

    EXPECT_TRUE(reg.IsValid(e1));
    EXPECT_TRUE(reg.IsValid(e2));
    EXPECT_EQ(e1, 0);
    EXPECT_EQ(e2, 1);

    reg.DestroyEntity(e1);
    EXPECT_FALSE(reg.IsValid(e1));
    EXPECT_TRUE(reg.IsValid(e2));

    EntityID e3 = reg.CreateEntity();
    EXPECT_EQ(e3, e1);
    EXPECT_TRUE(reg.IsValid(e3));
}

TEST(RegistryTest, Component_BasicCRUD) {
    Registry reg;
    EntityID e = reg.CreateEntity();

    reg.AddComponent<Position>(e, { 10.0f, 20.0f });
    EXPECT_TRUE(reg.HasComponent<Position>(e));
    EXPECT_FALSE(reg.HasComponent<Velocity>(e));

    Position& pos = reg.GetComponent<Position>(e);
    EXPECT_FLOAT_EQ(pos.x, 10.0f);
    EXPECT_FLOAT_EQ(pos.y, 20.0f);

    pos.x = 50.0f;
    const Position& constPos = reg.GetComponent<Position>(e);
    EXPECT_FLOAT_EQ(constPos.x, 50.0f);

    Position* ptrPos = reg.TryGetComponent<Position>(e);
    ASSERT_NE(ptrPos, nullptr);
    EXPECT_FLOAT_EQ(ptrPos->x, 50.0f);

    Velocity* ptrVel = reg.TryGetComponent<Velocity>(e);
    EXPECT_EQ(ptrVel, nullptr);

    reg.RemoveComponent<Position>(e);
    EXPECT_FALSE(reg.HasComponent<Position>(e));
    EXPECT_EQ(reg.TryGetComponent<Position>(e), nullptr);
}

TEST(RegistryTest, Component_DefaultInit) {
    Registry reg;
    EntityID e = reg.CreateEntity();

    reg.AddComponent<Health>(e);

    EXPECT_TRUE(reg.HasComponent<Health>(e));

    reg.GetComponent<Health>(e).hp = 100;
    EXPECT_EQ(reg.GetComponent<Health>(e).hp, 100);
}

TEST(RegistryTest, Multi_Variadic) {
    Registry reg;
    EntityID e = reg.CreateEntity();

    reg.AddComponents<Position, Velocity, Health>(
        e,
        Position{ 1, 2 },
        Velocity{ 0.5f, 0.5f },
        Health{ 100, 100 }
    );

    EXPECT_TRUE(reg.HasComponent<Position>(e));
    EXPECT_TRUE(reg.HasComponent<Velocity>(e));
    EXPECT_TRUE(reg.HasComponent<Health>(e));

    EXPECT_TRUE((reg.HasComponents<Position, Velocity>(e)));
    EXPECT_TRUE((reg.HasComponents<Position, Velocity, Health>(e)));
    EXPECT_FALSE((reg.HasComponents<Position, TagEnemy>(e)));

    auto [pos, vel] = reg.GetComponents<Position, Velocity>(e);
    EXPECT_FLOAT_EQ(pos.x, 1.0f);
    EXPECT_FLOAT_EQ(vel.dx, 0.5f);

    pos.x = 99.0f;
    vel.dx = 99.0f;

    EXPECT_FLOAT_EQ(reg.GetComponent<Position>(e).x, 99.0f);
    EXPECT_FLOAT_EQ(reg.GetComponent<Velocity>(e).dx, 99.0f);

    reg.RemoveComponents<Position, Health>(e);
    EXPECT_FALSE(reg.HasComponent<Position>(e));
    EXPECT_FALSE(reg.HasComponent<Health>(e));
    EXPECT_TRUE(reg.HasComponent<Velocity>(e));
}

TEST(RegistryTest, Destruction_Cleanup) {
    Registry reg;
    EntityID e = reg.CreateEntity();

    reg.AddComponents<Position, Velocity>(e, { 1,1 }, { 2,2 });

    EXPECT_TRUE(reg.HasComponent<Position>(e));
    EXPECT_TRUE(reg.HasComponent<Velocity>(e));

    reg.DestroyEntity(e);

    EXPECT_FALSE(reg.IsValid(e));
    EXPECT_FALSE(reg.HasComponent<Position>(e));
    EXPECT_FALSE(reg.HasComponent<Velocity>(e));
    EXPECT_EQ(reg.TryGetComponent<Position>(e), nullptr);

    EntityID e_new = reg.CreateEntity();
    EXPECT_EQ(e_new, e);

    EXPECT_FALSE(reg.HasComponent<Position>(e_new));
    EXPECT_FALSE(reg.HasComponent<Velocity>(e_new));
}

TEST(RegistryTest, DirectPoolAccess) {
    Registry reg;
    EntityID e = reg.CreateEntity();
    reg.AddComponent<Position>(e, { 5, 5 });

    auto* posPool = reg.GetPool<Position>();
    ASSERT_NE(posPool, nullptr);

    EXPECT_TRUE(posPool->Has(e));
    EXPECT_FLOAT_EQ(posPool->Get(e).x, 5.0f);

    posPool->Get(e).y = 10.0f;
    EXPECT_FLOAT_EQ(reg.GetComponent<Position>(e).y, 10.0f);
}

TEST(RegistryTest, Clear) {
    Registry reg;

    std::vector<EntityID> entities;
    for (int i = 0; i < 100; ++i) {
        EntityID e = reg.CreateEntity();
        reg.AddComponent<Position>(e);
        entities.push_back(e);
    }

    EXPECT_TRUE(reg.IsValid(entities[0]));
    EXPECT_TRUE(reg.HasComponent<Position>(entities[0]));

    reg.Clear();

    EXPECT_FALSE(reg.IsValid(entities[0]));
    EXPECT_FALSE(reg.IsValid(entities[99]));

    EntityID newE = reg.CreateEntity();
    EXPECT_FALSE(reg.HasComponent<Position>(newE));
}

TEST(RegistryTest, RecycleStrategy) {
    Registry reg;
    EntityID e0 = reg.CreateEntity(); // 0
    EntityID e1 = reg.CreateEntity(); // 1
    EntityID e2 = reg.CreateEntity(); // 2

    reg.DestroyEntity(e1);

    EntityID e1_new = reg.CreateEntity();
    EXPECT_EQ(e1_new, 1);

    EntityID e3 = reg.CreateEntity();
    EXPECT_EQ(e3, 3);
}