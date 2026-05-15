#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SystemGroupNames
    {
        static constexpr const char* UndefinedSystems = "UndefinedSystems";
        static constexpr const char* CoreSystems = "CoreSystems";
        static constexpr const char* DirectionLightSystems = "DirectionLightSystems";
        static constexpr const char* PointLightSystems = "PointLightSystems";
        static constexpr const char* SpotLightSystems = "SpotLightSystems";
        static constexpr const char* PhysicsSystems = "PhysicsSystems";
        static constexpr const char* RenderingSystems = "RenderingSystems";
    };
}