#include "FileSceneSource.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"

#include "Engine/Component/Components.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Component/TransformComponentSchema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"

namespace Syn
{
    FileSceneSource::FileSceneSource(std::filesystem::path path)
        : _path(std::move(path))
    {}

    bool FileSceneSource::Populate(Scene& scene)
    {
        auto serializer = ServiceLocator::GetSerializer();

        if (!serializer)
        {
            Error("FileSceneSource: Serializer not found in ServiceLocator!");
            return false;
        }
        /*
        using FullSceneSnapshot = SceneSnapshot<
            TransformComponent,
            CameraComponent,
            ModelComponent,
            AnimationComponent,
            PointLightComponent,
            PointLightShadowComponent,
            SpotLightComponent,
            SpotLightShadowComponent,
            DirectionLightComponent,
            DirectionLightShadowComponent,
            BoxColliderComponent,
            SphereColliderComponent,
            CapsuleColliderComponent,
            RigidBodyComponent,
            MaterialOverrideComponent
        >;
        */

        using FullSceneSnapshot = SceneSnapshot<
            TransformComponent
        >;

        FullSceneSnapshot snapshot{ scene };

        bool success = serializer->LoadFromFile(_path, snapshot);

        if (success) {
            Info("Successfully loaded scene from {}", _path.string());
        }
        else {
            Error("Failed to load scene from {}", _path.string());
        }

        return success;
    }
}