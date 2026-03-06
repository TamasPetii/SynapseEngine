#pragma once
#include "Engine/SynApi.h"
#include "GpuVertexData.h"
#include "GpuIndexedDrawData.h"
#include "GpuMeshletDrawData.h"
#include "GpuNodeTransform.h"

namespace Syn
{
    struct SYN_API GpuBatchedModel
    {
        GpuVertexData vertexData;
        GpuIndexedDrawData indexedData;
        GpuMeshletDrawData meshletData;
        std::vector<GpuNodeTransform> nodeTransforms;
    };
}

