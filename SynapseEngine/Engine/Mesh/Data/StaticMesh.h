#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"

namespace Syn
{
    struct SYN_API StaticMesh
    {
        CookedModel cpuData;
        //GpuMesh gpuData;
    };
}

