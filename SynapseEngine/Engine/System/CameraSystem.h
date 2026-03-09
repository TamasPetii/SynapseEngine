#pragma once
#include "ComponentSystem.h"
#include "Engine/Component/CameraComponent.h"

namespace Syn
{
    class SYN_API CameraSystem : public ComponentSystem<CameraComponent>
    {
    public:
        void OnUpdate(std::shared_ptr<Registry> registry, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ComponentBufferManager> componentBufferManager, uint32_t frameIndex, tf::Subflow& subflow) override;

        std::string GetName() const override { return "CameraSystem"; }

        static float ConvertDepthToLinearNormalized(float depth, float nearPlane, float farPlane);
    };
}