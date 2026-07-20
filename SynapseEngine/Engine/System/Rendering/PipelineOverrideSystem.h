#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Rendering/PipelineOverrideComponent.h"

namespace Syn
{
    class SYN_API PipelineOverrideSystem : public ComponentSystem<PipelineOverrideComponent>
    {
    public:
        std::string GetName() const override { return "PipelineOverrideSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::RenderingSystems; }

        std::vector<TypeID> GetWriteDependencies() const override;
    };
}