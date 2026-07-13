#pragma once
#include "Engine/SynApi.h"
#include "CullingSettings.h"
#include "LightingSettings.h"
#include "PostProcessSettings.h"
#include "DebugSettings.h"
#include "EnvironmentSettings.h"

namespace Syn
{
    struct SYN_API SceneSettings
    {
        SceneSettings();

        CullingSettings culling;
        LightingSettings lighting;
        PostProcessSettings postProcess;
        DebugSettings debug;
        EnvironmentSettings environment;
    };
}