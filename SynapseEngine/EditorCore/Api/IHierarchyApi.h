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