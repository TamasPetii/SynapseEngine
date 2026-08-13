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
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSettingsSchema.h"
#include "Engine/Serialization/Schema/Registry/PoolSchema.h"
#include "Engine/Serialization/Schema/Registry/DataMixinSchema.h"
#include "Engine/Serialization/Schema/Registry/SegmentedStorageImplSchema.h"
#include "Engine/Serialization/Schema/Registry/SparseVectorMappingSchema.h"
#include "Engine/Serialization/Schema/Registry/StorageBackendSchema.h"
#include "Engine/Serialization/Schema/Registry/RegistrySchema.h"
#include "Engine/Serialization/Schema/Registry/FlatStorageImplSchema.h"
#include "Engine/Serialization/Schema/Material/MaterialSchema.h"
#include "Engine/Serialization/Schema/Models/MaterialInfoSchema.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Scene/Scene.h"

namespace Syn
{
    struct SYN_API AnimationManifestEntry {
        std::string filePath;
        uint32_t localModelIndex;
    };

    template <>
    struct Schema<AnimationManifestEntry> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& entry = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("filePath", entry.filePath);
            ar.Property("localModelIndex", entry.localModelIndex);
        }
    };

    struct SYN_API MaterialManifestEntry {
        std::string name;
        std::string path;
        Material material;
    };

    template <>
    struct Schema<MaterialManifestEntry> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& entry = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", entry.name);
            ar.Property("path", entry.path);
            ar.Property("material", entry.material);
        }
    };

    struct SYN_API TextureManifestEntry {
        std::string name;
        TexturePayload payload;
    };

    template <>
    struct Schema<TextureManifestEntry> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& entry = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", entry.name);
            ar.Property("payload", entry.payload);
        }
    };

    struct SYN_API VideoManifestEntry {
        std::string name;
        std::string path;
    };

    template <>
    struct Schema<VideoManifestEntry> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& entry = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("name", entry.name);
            ar.Property("path", entry.path);
        }
    };

    template <typename... Components>
    struct SceneSnapshot
    {
        Scene& scene;
        std::vector<std::string> modelManifest;
        std::vector<AnimationManifestEntry> animationManifest;
        std::vector<MaterialManifestEntry> materialManifest;
        std::vector<TextureManifestEntry> textureManifest;
        std::vector<VideoManifestEntry> videoManifest;
    };

    template <typename... Components>
    struct Schema<SceneSnapshot<Components...>>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);

            auto& snapshot = const_cast<std::remove_const_t<U>&>(val);
            Scene& scene = snapshot.scene;

            SceneSettings& settings = SceneInsider::GetSettings(scene, SceneInsider::GetKey());
            Registry& registry = SceneInsider::GetRegistry(scene, SceneInsider::GetKey());
            EntityID& sceneCam = SceneInsider::GetSceneCameraEntity(scene, SceneInsider::GetKey());
            EntityID& debugCam = SceneInsider::GetDebugCameraEntity(scene, SceneInsider::GetKey());

            ar.Property("Settings", settings);

            ar.Property("SceneCameraEntity", sceneCam);
            ar.Property("DebugCameraEntity", debugCam);

            ar.Property("ModelManifest", snapshot.modelManifest);
            ar.Property("AnimationManifest", snapshot.animationManifest);
            ar.Property("MaterialManifest", snapshot.materialManifest); 
            ar.Property("TextureManifest", snapshot.textureManifest);
            ar.Property("VideoManifest", snapshot.videoManifest);

            RegistrySnapshot<Components...> regSnap{ registry };
            ar.Property("Registry", regSnap);
        }
    };
}