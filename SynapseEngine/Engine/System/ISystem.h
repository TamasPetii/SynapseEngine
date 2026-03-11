#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Registry/Type/TypeInfo.h"
#include <vector>
#include <string>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>
#include "Engine/Scene/Scene.h"

namespace Syn
{
    class SYN_API ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) = 0;
        virtual void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) = 0;
        virtual void OnFinish(Scene* scene, tf::Subflow& subflow) = 0;

        virtual std::vector<TypeID> GetReadDependencies() const { return {}; }
        virtual std::vector<TypeID> GetWriteDependencies() const { return {}; }
        virtual std::string GetName() const = 0;
    };
}