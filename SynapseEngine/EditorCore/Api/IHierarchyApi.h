#pragma once
#include <string>
#include <vector>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class IHierarchyApi {
    public:
        virtual ~IHierarchyApi() = default;

        virtual std::vector<EntityID> GetRootEntities() const = 0;
        virtual std::vector<EntityID> GetChildren(EntityID entity) const = 0;

        virtual std::string GetEntityIcon(EntityID entity) const = 0;
        virtual bool HasChildren(EntityID entity) const = 0;

        virtual void SetParent(EntityID child, EntityID parent) = 0;

        virtual EntityID CreateEntity(const std::string& name, EntityID parent = NULL_ENTITY) = 0;
        virtual void DestroyEntity(EntityID entity) = 0;
    };
}