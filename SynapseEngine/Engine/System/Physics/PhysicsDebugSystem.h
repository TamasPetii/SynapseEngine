#pragma once
#include "Engine/System/ISystem.h"
#include "Engine/Physics/PhysicsTypes.h"
#include <vector>

namespace Syn
{
    class SYN_API PhysicsDebugSystem : public ISystem
    {
    public:
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        std::string GetName() const override { return "PhysicsDebugSystem"; }

        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
    private:
        std::vector<PhysicsDebugVertex> _debugLines;
    };
}