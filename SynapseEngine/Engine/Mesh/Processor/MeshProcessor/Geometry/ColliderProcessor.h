#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"
#include <taskflow/taskflow.hpp>

namespace Syn
{
    class SYN_API ColliderProcessor : public IMeshProcessor
    {
    public:
        virtual ~ColliderProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    private:
        void ComputeMeshLocalBounds(CookedMesh& mesh);
        void ComputeMeshletBoundsJob(CookedMesh& mesh, CookedMeshLod& lod, CookedMeshlet& meshlet);
        void ComputeGlobalBounds(CookedModel& cookedModel, tf::Taskflow& taskflow);
    };
}