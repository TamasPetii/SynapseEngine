#pragma once
#include "Engine/SynApi.h"
#include "IMeshletProcessor.h"

namespace Syn
{
    class SYN_API MeshoptimizerMeshletProcessor : public IMeshletProcessor
    {
    public:
        MeshoptimizerMeshletProcessor(size_t maxVertices = 64, size_t maxTriangles = 124, float coneWeight = 0.0f);
        virtual ~MeshoptimizerMeshletProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    private:
        size_t _maxVertices;
        size_t _maxTriangles;
        float _coneWeight;
    };
}