#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"

namespace Syn
{
    class SYN_API MaterialOverrideSystem : public ComponentSystem<MaterialOverrideComponent>
    {
    public:
        std::string GetName() const override { return "MaterialOverrideSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::RenderingSystems; }

        std::vector<TypeID> GetWriteDependencies() const override;
    };
}