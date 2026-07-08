#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Core/HierarchyComponent.h"

namespace Syn
{
    class SYN_API TransformSetupSystem : public ComponentSystem<TransformComponent>
    {
    public:
        std::string GetName() const override { return "TransformSetupSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
    };
}