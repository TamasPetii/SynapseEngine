#pragma once
#include "ComponentSystem.h"
#include "Engine/Component/CameraComponent.h"

namespace Syn
{
    class SYN_API CameraSystem : public ComponentSystem<CameraComponent>
    {
    public:
        std::string GetName() const override { return "CameraSystem"; }
    protected:
        std::string GetSparseBufferName() const override { return BufferNames::CameraSparseMap; }
    protected:
        void UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic) override;
    };
}