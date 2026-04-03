#pragma once
#include "Engine/System/ISystem.h"

namespace Syn
{
    class SYN_API PhysicsSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "PhysicsSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
    };
}