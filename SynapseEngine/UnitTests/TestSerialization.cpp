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

#include "gtest/gtest.h"
#include <filesystem>
#include <vector>

#include "Engine/Serialization/Serializer.h"
#include "Engine/Serialization/Archive/DefaultArchiveRegistry.h"
#include "Engine/Serialization/Archive/Output/Json/NlohmannJsonOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Json/NlohmannJsonInputArchive.h"
#include "Engine/Serialization/Archive/Input/Binary/BinaryInputArchive.h"
#include "Engine/Serialization/Archive/Output/Binary/BinaryOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Xml/TinyXmlInputArchive.h"
#include "Engine/Serialization/Archive/Output/Xml/TinyXmlOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Yaml/YamlCppInputArchive.h"
#include "Engine/Serialization/Archive/Output/Yaml/YamlCppOutputArchive.h"
#include "Engine/Serialization/Archive/Input/Toml/PlusPlusTomlInputArchive.h"
#include "Engine/Serialization/Archive/Output/Toml/PlusPlusTomlOutputArchive.h"

#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Serialization/Schema/Component/Core/TransformComponentSchema.h"

#include "Engine/Registry/Pool/Pool.h"
#include "Engine/Registry/Pool/Storage/SynStorage.h"
#include "Engine/Registry/Pool/Mapping/SynMapping.h"
#include "Engine/Registry/Pool/Mapping/Extension/SparseVectorMappingExtension.h"
#include "Engine/Registry/Pool/Storage/Extension/SegmentedStorageImplExtension.h"

#include "Engine/Serialization/Schema/Registry/PoolSchema.h"
#include "Engine/Serialization/Schema/Registry/DataMixinSchema.h"
#include "Engine/Serialization/Schema/Registry/SegmentedStorageImplSchema.h"
#include "Engine/Serialization/Schema/Registry/SparseVectorMappingSchema.h"
#include "Engine/Serialization/Schema/Registry/StorageBackendSchema.h"
#include "Engine/Serialization/Schema/Registry/RegistrySchema.h"
#include "Engine/Serialization/Schema/Registry/FlatStorageImplSchema.h"

#include "TestComponents.h"
#include "TestComponentsSchema.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Settings/SceneSettings.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSettingsSchema.h"
#include "Engine/Serialization/Schema/Material/MaterialSchema.h"

#include "Engine/Component/Components.h"
#include "Engine/EnginePaths.h"

using namespace Syn;

class SerializationTest : public ::testing::Test {
protected:
    std::unique_ptr<Serializer> serializer;
    std::filesystem::path saveDir;

    void SetUp() override {
        auto registry = std::make_unique<DefaultArchiveRegistry>();
        registry->RegisterOutputAuto<NlohmannJsonOutputArchive>(10);
        registry->RegisterInputAuto<NlohmannJsonInputArchive>(10);
        registry->RegisterOutputAuto<BinaryOutputArchive>(10);
        registry->RegisterInputAuto<BinaryInputArchive>(10);
        registry->RegisterOutputAuto<TinyXmlOutputArchive>(10);
        registry->RegisterInputAuto<TinyXmlInputArchive>(10);
        registry->RegisterOutputAuto<YamlCppOutputArchive>(10);
        registry->RegisterInputAuto<YamlCppInputArchive>(10);
        registry->RegisterOutputAuto<PlusPlusTomlOutputArchive>(10);
        registry->RegisterInputAuto<PlusPlusTomlInputArchive>(10);

        auto service = std::make_unique<DefaultSerializationService>(std::move(registry));
        serializer = std::make_unique<Serializer>(std::move(service));

        saveDir = Syn::EnginePaths::GetTestSavesDir();

        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directories(saveDir);
        }
    }

    TransformComponent CreateDummyTransform(float xOffset = 0.0f) {
        TransformComponent t;
        t.translation = glm::vec3(100.5f + xOffset, 50.0f, -25.2f);
        t.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
        t.scale = glm::vec3(2.0f, 2.0f, 2.0f);
        return t;
    }
};

TEST_F(SerializationTest, SingleTransform_AllFormats) {
    TransformComponent original = CreateDummyTransform();

    auto runFormatTest = [&](const std::string& extension) {
        std::filesystem::path path = saveDir / ("single_transform" + extension);

        EXPECT_TRUE(serializer->SaveToFile(path, original)) << "Failed to save " << extension;

        TransformComponent loaded;
        EXPECT_TRUE(serializer->LoadFromFile(path, loaded)) << "Failed to load " << extension;

        EXPECT_FLOAT_EQ(loaded.translation.x, original.translation.x);
        EXPECT_FLOAT_EQ(loaded.rotation.x, original.rotation.x);
        EXPECT_FLOAT_EQ(loaded.scale.z, original.scale.z);
        };

    runFormatTest(".json");
    runFormatTest(".bin");
    runFormatTest(".xml");
    runFormatTest(".yaml");
    runFormatTest(".toml");
}

TEST_F(SerializationTest, Vector10k_PerformanceAndIntegrity) {
    const int NUM_ELEMENTS = 10000;
    std::vector<TransformComponent> transformsToSave;
    transformsToSave.reserve(NUM_ELEMENTS);

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        transformsToSave.push_back(CreateDummyTransform(static_cast<float>(i)));
    }

    auto runVectorTest = [&](const std::string& extension, bool useBlit = false) {
        std::filesystem::path path = saveDir / ("vec10k" + extension);

        if (useBlit) {
            BlitVector<TransformComponent> saveArray{ transformsToSave };
            EXPECT_TRUE(serializer->SaveToFile(path, saveArray)) << "Failed to save FAST " << extension;

            std::vector<TransformComponent> loaded;
            BlitVector<TransformComponent> loadArray{ loaded };
            EXPECT_TRUE(serializer->LoadFromFile(path, loadArray)) << "Failed to load FAST " << extension;

            ASSERT_EQ(loaded.size(), NUM_ELEMENTS);
            EXPECT_FLOAT_EQ(loaded.back().translation.x, transformsToSave.back().translation.x);
        }
        else {
            EXPECT_TRUE(serializer->SaveToFile(path, transformsToSave)) << "Failed to save " << extension;

            std::vector<TransformComponent> loaded;
            EXPECT_TRUE(serializer->LoadFromFile(path, loaded)) << "Failed to load " << extension;

            ASSERT_EQ(loaded.size(), NUM_ELEMENTS);
            EXPECT_FLOAT_EQ(loaded.back().translation.x, transformsToSave.back().translation.x);
        }
        };

    runVectorTest(".json");
    runVectorTest(".xml");
    runVectorTest(".yaml");
    runVectorTest(".toml");
    runVectorTest("_slow.bin", false);
    runVectorTest("_fast.bin", true);
}

TEST_F(SerializationTest, Pool_BinaryBlit_Integrity) {
    using TestPool = Pool<TransformComponent, SegmentedStorage<TransformComponent>, SparseVectorMapping>;
    TestPool originalPool;

    EntityID e1 = 5;
    EntityID e2 = 42;
    EntityID e3 = 1024;

    originalPool.Add(e1, CreateDummyTransform(5.0f));
    originalPool.Add(e2, CreateDummyTransform(42.0f));
    originalPool.Add(e3, CreateDummyTransform(1024.0f));

    originalPool.SetCategory(e1, StorageCategory::Static);
    originalPool.SetCategory(e2, StorageCategory::Dynamic);

    EXPECT_EQ(originalPool.Size(), 3);
    EXPECT_TRUE(originalPool.Has(e2));

    std::filesystem::path poolPath = saveDir / "isolated_pool.bin";
    EXPECT_TRUE(serializer->SaveToFile(poolPath, originalPool));

    TestPool loadedPool;
    EXPECT_TRUE(serializer->LoadFromFile(poolPath, loadedPool));
    ASSERT_EQ(loadedPool.Size(), 3);

    EXPECT_TRUE(loadedPool.Has(e1));
    EXPECT_TRUE(loadedPool.Has(e2));
    EXPECT_TRUE(loadedPool.Has(e3));
    EXPECT_FALSE(loadedPool.Has(99));

    EXPECT_FLOAT_EQ(loadedPool.Get(e2).translation.x, 100.5f + 42.0f);
    EXPECT_FLOAT_EQ(loadedPool.Get(e3).translation.x, 100.5f + 1024.0f);

    EXPECT_EQ(loadedPool.GetCategory(e1), StorageCategory::Static);
    EXPECT_EQ(loadedPool.GetCategory(e2), StorageCategory::Dynamic);
    EXPECT_EQ(loadedPool.GetCategory(e3), StorageCategory::Stream);
}

TEST_F(SerializationTest, Registry100_AllFormats) {
    Registry originalReg;
    const int NUM_ENTITIES = 100;

    for (int i = 0; i < NUM_ENTITIES; ++i) {
        EntityID e = originalReg.CreateEntity();

        originalReg.AddComponent<TransformComponent>(e, CreateDummyTransform(static_cast<float>(i)));

        if (i % 2 == 0) {
            originalReg.AddComponent<Velocity>(e, Velocity{ static_cast<float>(i), 0.0f });
        }

        if (i % 3 == 0) {
            originalReg.AddComponent<Health>(e, Health{ i, 100 });
        }

        if (i % 10 == 0) {
            originalReg.AddComponent<TagEnemy>(e);
        }
    }

    originalReg.DestroyEntity(5);
    originalReg.DestroyEntity(50);
    originalReg.DestroyEntity(95);

    using TestSnapshot = RegistrySnapshot<TransformComponent, Velocity, Health, TagEnemy>;

    auto runRegistryTest = [&](const std::string& extension) {
        std::filesystem::path path = saveDir / ("registry100" + extension);

        TestSnapshot saveSnapshot{ originalReg };
        EXPECT_TRUE(serializer->SaveToFile(path, saveSnapshot)) << "Failed to save Registry as " << extension;

        Registry loadedReg;
        TestSnapshot loadSnapshot{ loadedReg };
        EXPECT_TRUE(serializer->LoadFromFile(path, loadSnapshot)) << "Failed to load Registry from " << extension;

        EXPECT_FALSE(loadedReg.IsValid(5));
        EXPECT_FALSE(loadedReg.IsValid(50));
        EXPECT_FALSE(loadedReg.IsValid(95));

        EXPECT_TRUE(loadedReg.IsValid(4));
        EXPECT_TRUE(loadedReg.IsValid(99));

        EXPECT_TRUE(loadedReg.HasComponent<TransformComponent>(4));
        EXPECT_TRUE(loadedReg.HasComponent<Velocity>(4));
        EXPECT_FALSE(loadedReg.HasComponent<Health>(4));
        EXPECT_FALSE(loadedReg.HasComponent<TagEnemy>(4));

        EXPECT_FLOAT_EQ(loadedReg.GetComponent<TransformComponent>(4).translation.x, 100.5f + 4.0f);
        EXPECT_FLOAT_EQ(loadedReg.GetComponent<Velocity>(4).dx, 4.0f);

        EXPECT_TRUE((loadedReg.HasComponents<TransformComponent, Velocity, Health, TagEnemy>(30)));
        EXPECT_EQ(loadedReg.GetComponent<Health>(30).hp, 30);

        EntityID newEntity = loadedReg.CreateEntity();
        EXPECT_TRUE(newEntity == 5 || newEntity == 50 || newEntity == 95);
        };

    runRegistryTest(".bin");
    runRegistryTest(".xml");
    runRegistryTest(".toml");
    runRegistryTest(".json");
    runRegistryTest(".yaml");
}

TEST_F(SerializationTest, SceneSnapshot_AllFormats) {
    Scene originalScene(1, nullptr, false);

    SceneSettings* originalSettings = originalScene.GetSettings();
    originalSettings->postProcess.bloomThreshold = 3.14f;
    originalSettings->postProcess.enableBloom = false;
    originalSettings->lighting.ambientStrength = 0.88f;
    originalSettings->lighting.pipelineType = PipelineType::Deferred;
    originalSettings->culling.enableMeshletConeCulling = false;

    Registry* originalReg = originalScene.GetRegistry();

    EntityID e1 = originalReg->CreateEntity();
    originalReg->AddComponent<TransformComponent>(e1, CreateDummyTransform(10.0f));

    EntityID e2 = originalReg->CreateEntity();
    originalReg->AddComponent<TransformComponent>(e2, CreateDummyTransform(20.0f));

    using TestSnapshot = SceneSnapshot<TransformComponent>;

    auto runSceneTest = [&](const std::string& extension) {
        std::filesystem::path path = saveDir / ("scene_full" + extension);

        TestSnapshot saveSnapshot{ originalScene };
        EXPECT_TRUE(serializer->SaveToFile(path, saveSnapshot)) << "Failed to save Scene as " << extension;

        Scene loadedScene(1, nullptr, false);
        TestSnapshot loadSnapshot{ loadedScene };
        EXPECT_TRUE(serializer->LoadFromFile(path, loadSnapshot)) << "Failed to load Scene from " << extension;

        SceneSettings* loadedSettings = loadedScene.GetSettings();
        EXPECT_FLOAT_EQ(loadedSettings->postProcess.bloomThreshold, 3.14f);
        EXPECT_FALSE(loadedSettings->postProcess.enableBloom);
        EXPECT_FLOAT_EQ(loadedSettings->lighting.ambientStrength, 0.88f);
        EXPECT_EQ(loadedSettings->lighting.pipelineType, PipelineType::Deferred);
        EXPECT_FALSE(loadedSettings->culling.enableMeshletConeCulling);

        Registry* loadedReg = loadedScene.GetRegistry();

        EXPECT_TRUE(loadedReg->IsValid(e1));
        EXPECT_TRUE(loadedReg->IsValid(e2));

        EXPECT_FLOAT_EQ(loadedReg->GetComponent<TransformComponent>(e1).translation.x, 100.5f + 10.0f);
        EXPECT_FLOAT_EQ(loadedReg->GetComponent<TransformComponent>(e2).translation.x, 100.5f + 20.0f);
        };

    runSceneTest(".json");
    runSceneTest(".bin");
    runSceneTest(".xml");
    runSceneTest(".yaml");
    runSceneTest(".toml");
}