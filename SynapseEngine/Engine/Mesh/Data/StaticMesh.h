#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuBatchedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuModelBuffers.h"

namespace Syn
{
    struct SYN_API StaticMesh
    {
        CookedModel cpuData;
        GpuBatchedModel gpuData;
        GpuModelBuffers hardwareBuffers;

        std::vector<uint32_t> meshMaterialIndices;
        std::vector<MeshDrawBlueprint> baseDrawCommands;
    };
}

