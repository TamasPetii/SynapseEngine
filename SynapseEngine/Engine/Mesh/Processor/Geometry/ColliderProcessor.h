#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API ColliderProcessor : public IMeshProcessor
    {
    public:
        virtual ~ColliderProcessor() override = default;
        virtual void Process(CookedModel& cookedModel) override;
    private:
        void ComputeMeshLocalBounds(CookedMesh& mesh);
        void ComputeMeshletBounds(CookedMesh& mesh);
        void ComputeGlobalBounds(CookedModel& cookedModel);
    };
}