#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Serialization/Schema/Component/TransformComponentSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSettingsSchema.h"
#include "Engine/Serialization/Schema/Registry/PoolSchema.h"
#include "Engine/Serialization/Schema/Registry/DataMixinSchema.h"
#include "Engine/Serialization/Schema/Registry/SegmentedStorageImplSchema.h"
#include "Engine/Serialization/Schema/Registry/SparseVectorMappingSchema.h"
#include "Engine/Serialization/Schema/Registry/StorageBackendSchema.h"
#include "Engine/Serialization/Schema/Registry/RegistrySchema.h"
#include "Engine/Serialization/Schema/Registry/FlatStorageImplSchema.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Scene/Scene.h"

namespace Syn
{
    template <typename... Components>
    struct SYN_API SceneSnapshot
    {
        Scene& scene;
    };

    template <typename... Components>
    struct SYN_API Schema<SceneSnapshot<Components...>>
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

            RegistrySnapshot<Components...> regSnap{ registry };
            ar.Property("Registry", regSnap);
        }
    };
}