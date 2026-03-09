#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Registry/Type/TypeInfo.h"
#include <vector>
#include <string>

#include "Engine/Manager/ComponentBufferManager.h"

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    class SYN_API ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void OnUpdate(std::shared_ptr<Registry> registry, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) = 0;
        virtual void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ComponentBufferManager> componentBufferManager, uint32_t frameIndex, tf::Subflow& subflow) = 0;
        virtual void OnFinish(std::shared_ptr<Registry> registry, tf::Subflow& subflow) = 0;

        virtual std::vector<TypeID> GetReadDependencies() const { return {}; }
        virtual std::vector<TypeID> GetWriteDependencies() const { return {}; }
        virtual std::string GetName() const = 0;
    };
}