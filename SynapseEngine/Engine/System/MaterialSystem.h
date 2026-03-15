#pragma once
#include "Engine/SynApi.h"
#include "Engine/System/ComponentSystem.h"
#include "Engine/Component/ModelComponent.h"
#include <vector>

namespace Syn
{
    class SYN_API MaterialSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "MaterialSystem"; }
        std::vector<TypeID> GetWriteDependencies() const override;
    protected:
        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
    private:
        bool _needsUpload = false;
        uint32_t _lastModelManagerVersion = 0;
        std::vector<uint32_t> _flatMaterialIndices;

        uint32_t _framesToUpload = 0;
    };
}