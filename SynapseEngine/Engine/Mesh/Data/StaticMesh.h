#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuModelBuffers.h"
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    struct SYN_API StaticMesh
    {
        CpuModelData cpuData;
        GpuModelBuffers hardwareBuffers;

        std::unique_ptr<CookedModel> transientCpuData;
        std::unique_ptr<GpuBatchedModel> transientGpuData;
    };
}

