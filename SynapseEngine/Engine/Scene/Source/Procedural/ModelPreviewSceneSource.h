#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API ModelPreviewSceneSource : public ISceneSource
    {
    public:
        ModelPreviewSceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}