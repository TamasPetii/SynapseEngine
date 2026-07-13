#pragma once
#include "IApi.h"
#include <string>
#include <vector>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn 
{
    enum class EntityTemplate {
        Empty,
        Camera,
        DirectionalLight,
        PointLight,
        SpotLight,
        BoxCollider,
        SphereCollider,
        CapsuleCollider,
        ConvexCollider,
        MeshCollider,
        ShapeCube,
        ShapeSphere,
        ShapeQuad,
        ShapeCylinder,
        ShapeCone,
        ShapeCapsule,
        ShapeHemisphere,
        ShapePyramid,
        ShapeGrid,
        ShapeTorus,
        Model,
        Animation
    };

    class IHierarchyApi : public IApi {
    public:
        virtual ~IHierarchyApi() = default;

        virtual std::vector<EntityID> GetRootEntities() const = 0;
        virtual std::vector<EntityID> GetChildren(EntityID entity) const = 0;

        virtual std::string GetEntityIcon(EntityID entity) const = 0;
        virtual bool HasChildren(EntityID entity) const = 0;

        virtual EntityID GetParent(EntityID entity) const = 0;
        virtual void SetParent(EntityID child, EntityID parent) = 0;

        virtual EntityID CreateEntity(EntityTemplate templateType, EntityID parent = NULL_ENTITY) = 0;
        virtual void DestroyEntityRecursive(EntityID entity) = 0;
        virtual void DestroyEntityKeepChildren(EntityID entity) = 0;
        virtual EntityID CopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) = 0;
        virtual EntityID FullCopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) = 0;
        virtual void DestroyEntity(EntityID entity) = 0;

        virtual uint64_t GetVersion() const = 0;
    };
}