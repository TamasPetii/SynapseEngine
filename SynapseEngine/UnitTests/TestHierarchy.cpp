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

#include <gtest/gtest.h>
#include "Engine/Registry/Registry.h"
#include "Engine/Scene/Hierarchy/HierarchyManager.h"
#include "Engine/Component/Core/HierarchyComponent.h"

using namespace Syn;

class HierarchyTest : public ::testing::Test {
protected:
    Registry reg;
    std::unique_ptr<HierarchyManager> hm;

    void SetUp() override {
		reg.EnsurePool<HierarchyComponent>();
        hm = std::make_unique<HierarchyManager>(&reg);
    }

    EntityID CreateNode() {
        EntityID e = reg.CreateEntity();
        hm->OnEntityCreated(e);
        return e;
    }

    HierarchyComponent& GetComp(EntityID e) {
        return reg.GetPool<HierarchyComponent>()->Get(e);
    }
};

// Tests basic parent-child attachment and verifies that topological depth levels and structural pointers are correctly assigned.
TEST_F(HierarchyTest, BasicAttachAndDepth) {
    EntityID root = CreateNode();
    EntityID child = CreateNode();
    EntityID grandChild = CreateNode();

    hm->AttachChild(root, child);
    hm->AttachChild(child, grandChild);

    EXPECT_EQ(GetComp(root).depthLevel, 0);
    EXPECT_EQ(GetComp(child).depthLevel, 1);
    EXPECT_EQ(GetComp(grandChild).depthLevel, 2);

    EXPECT_EQ(GetComp(root).firstChild, child);
    EXPECT_EQ(GetComp(child).parent, root);
    EXPECT_EQ(GetComp(child).firstChild, grandChild);
    EXPECT_EQ(GetComp(grandChild).parent, child);

    auto level1 = hm->GetEntitiesInLevel(1);
    ASSERT_EQ(level1.size(), 1);
    EXPECT_EQ(level1[0], child);
}

// Verifies the integrity of the doubly-linked sibling chain when attaching multiple children to the same parent.
TEST_F(HierarchyTest, SiblingChainIntegrity) {
    EntityID parent = CreateNode();
    EntityID c1 = CreateNode();
    EntityID c2 = CreateNode();
    EntityID c3 = CreateNode();

    hm->AttachChild(parent, c1);
    hm->AttachChild(parent, c2);
    hm->AttachChild(parent, c3);

    EXPECT_EQ(GetComp(parent).firstChild, c3);

    EXPECT_EQ(GetComp(c3).nextSibling, c2);
    EXPECT_EQ(GetComp(c2).prevSibling, c3);
    EXPECT_EQ(GetComp(c2).nextSibling, c1);
    EXPECT_EQ(GetComp(c1).prevSibling, c2);
    EXPECT_EQ(GetComp(c1).nextSibling, NULL_ENTITY);
    EXPECT_EQ(GetComp(c3).prevSibling, NULL_ENTITY);
}

// Tests detaching a child from the middle of a sibling chain, ensuring pointers are rerouted and the child returns to the root level.
TEST_F(HierarchyTest, DetachMiddleSibling) {
    EntityID parent = CreateNode();
    EntityID c1 = CreateNode();
    EntityID c2 = CreateNode();
    EntityID c3 = CreateNode();

    hm->AttachChild(parent, c1);
    hm->AttachChild(parent, c2);
    hm->AttachChild(parent, c3);

    hm->DetachChild(c2);

    EXPECT_EQ(GetComp(c3).nextSibling, c1);
    EXPECT_EQ(GetComp(c1).prevSibling, c3);

    EXPECT_EQ(GetComp(c2).parent, NULL_ENTITY);
    EXPECT_EQ(GetComp(c2).prevSibling, NULL_ENTITY);
    EXPECT_EQ(GetComp(c2).nextSibling, NULL_ENTITY);
    EXPECT_EQ(GetComp(c2).depthLevel, 0);
}

// Validates the O(1) swap-and-pop removal logic, ensuring the contiguous memory block remains tightly packed and topological indices are updated.
TEST_F(HierarchyTest, SwapAndPopIntegrity) {
    EntityID parent = CreateNode();
    EntityID c1 = CreateNode();
    EntityID c2 = CreateNode();
    EntityID c3 = CreateNode();
    EntityID c4 = CreateNode();

    hm->AttachChild(parent, c1);
    hm->AttachChild(parent, c2);
    hm->AttachChild(parent, c3);
    hm->AttachChild(parent, c4);

    hm->DetachChild(c2);

    auto level1 = hm->GetEntitiesInLevel(1);
    ASSERT_EQ(level1.size(), 3);

    bool c4Found = false;
    for (uint32_t i = 0; i < level1.size(); ++i) {
        if (level1[i] == c4) {
            EXPECT_EQ(GetComp(c4).topoIndex, hm->GetLevels()[1].startIndex + i);
            c4Found = true;
        }
    }
    EXPECT_TRUE(c4Found);
}

// Tests migrating an entire subtree to a new parent, ensuring the BFS algorithm correctly shifts the topological depth of all descendants.
TEST_F(HierarchyTest, SubtreeMigration) {
    EntityID rootA = CreateNode();
    EntityID childA = CreateNode();
    EntityID grandChildA = CreateNode();
    hm->AttachChild(rootA, childA);
    hm->AttachChild(childA, grandChildA);

    EntityID rootB = CreateNode();
    EntityID childB = CreateNode();
    hm->AttachChild(rootB, childB);

    hm->AttachChild(childB, rootA);

    EXPECT_EQ(GetComp(rootB).depthLevel, 0);
    EXPECT_EQ(GetComp(childB).depthLevel, 1);
    EXPECT_EQ(GetComp(rootA).depthLevel, 2);
    EXPECT_EQ(GetComp(childA).depthLevel, 3);
    EXPECT_EQ(GetComp(grandChildA).depthLevel, 4);

    auto level4 = hm->GetEntitiesInLevel(4);
    ASSERT_EQ(level4.size(), 1);
    EXPECT_EQ(level4[0], grandChildA);
}

// Triggers a dynamic capacity rebuild by adding many entities, verifying that memory reallocation preserves structural integrity and indices.
TEST_F(HierarchyTest, DynamicCapacityRebuildTrigger) {
    EntityID parent = CreateNode();
    std::vector<EntityID> children;

    for (int i = 0; i < 150; i++) {
        EntityID c = CreateNode();
        children.push_back(c);
        hm->AttachChild(parent, c);
    }

    auto level1 = hm->GetEntitiesInLevel(1);
    ASSERT_EQ(level1.size(), 150);

    for (EntityID c : children) {
        auto& comp = GetComp(c);
        EXPECT_EQ(comp.depthLevel, 1);
        EXPECT_EQ(comp.parent, parent);
        EXPECT_NE(comp.topoIndex, 0xFFFFFFFF);
    }
}

// Tests the lifecycle hook for entity destruction, ensuring all immediate children are safely orphaned and returned to the root level.
TEST_F(HierarchyTest, OnEntityDestroyedOrphaning) {
    EntityID root = CreateNode();
    EntityID c1 = CreateNode();
    EntityID c2 = CreateNode();

    hm->AttachChild(root, c1);
    hm->AttachChild(root, c2);

    hm->OnEntityDestroyed(root);
    reg.DestroyEntity(root);

    EXPECT_FALSE(reg.IsValid(root));

    EXPECT_EQ(GetComp(c1).parent, NULL_ENTITY);
    EXPECT_EQ(GetComp(c2).parent, NULL_ENTITY);

    EXPECT_EQ(GetComp(c1).depthLevel, 0);
    EXPECT_EQ(GetComp(c2).depthLevel, 0);

    EXPECT_EQ(GetComp(c1).prevSibling, NULL_ENTITY);
    EXPECT_EQ(GetComp(c2).nextSibling, NULL_ENTITY);
}

// Tests cascade orphaning when a node in the middle of the hierarchy is destroyed, ensuring its descendants are safely decoupled.
TEST_F(HierarchyTest, DestroyMiddleOfTree) {
    EntityID root = CreateNode();
    EntityID mid = CreateNode();
    EntityID leaf = CreateNode();

    hm->AttachChild(root, mid);
    hm->AttachChild(mid, leaf);

    hm->OnEntityDestroyed(mid);
    reg.DestroyEntity(mid);

    EXPECT_EQ(GetComp(root).firstChild, NULL_ENTITY);

    EXPECT_EQ(GetComp(leaf).parent, NULL_ENTITY);
    EXPECT_EQ(GetComp(leaf).depthLevel, 0);
}