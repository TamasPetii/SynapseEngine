#pragma once
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include <vector>

namespace Syn
{
    class SYN_API SelectionOutlineSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "SelectionOutlineSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;

        std::vector<TypeID> GetWriteDependencies() const override;
    private:
        EntityID _lastSelectedEntity = NULL_ENTITY;
        uint64_t _lastHierarchyVersion = 0;
        std::vector<uint32_t> _selectionMask;
    };
}