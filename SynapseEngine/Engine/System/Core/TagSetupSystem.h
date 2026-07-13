#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Core/HierarchyComponent.h"

namespace Syn
{
    class SYN_API TagSetupSystem : public ComponentSystem<TagComponent>
    {
    public:
        std::string GetName() const override { return "TagSetupSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
    };
}