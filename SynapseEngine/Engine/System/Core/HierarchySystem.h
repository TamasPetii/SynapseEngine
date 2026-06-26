#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Core/HierarchyComponent.h"

namespace Syn
{
    class SYN_API HierarchySystem : public ComponentSystem<HierarchyComponent>
    {
    public:
        HierarchySystem() = default;
        virtual ~HierarchySystem() = default;

        std::string GetName() const override { return "HierarchySystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::HierarchySparseMap; }
    };
}