#pragma once
#include "ISystem.h"

namespace Syn
{
    class SYN_API TransformSystem : public ISystem
    {
    public:
        void OnUpdate(std::shared_ptr<Registry> registry, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ComponentBufferManager> componentBufferManager, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(std::shared_ptr<Registry> registry, tf::Subflow& subflow) override;

        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override;
        std::string GetName() const override;
    };
}