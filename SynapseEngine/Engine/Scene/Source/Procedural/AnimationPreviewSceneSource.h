#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API AnimationPreviewSceneSource : public ISceneSource
    {
    public:
        AnimationPreviewSceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}