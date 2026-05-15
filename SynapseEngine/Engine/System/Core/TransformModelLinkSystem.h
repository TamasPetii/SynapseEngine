#pragma once
#include "Engine/System/ISystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"

namespace Syn
{
    class SYN_API TransformModelLinkSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "TransformModelLinkSystem"; }
        std::string GetGroup() const override { return SystemGroupNames::CoreSystems; }

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;

        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
    private:
        uint32_t _globalLinkVersion = 1;
        std::vector<uint32_t> _gpuLinkVersions;
    };
}