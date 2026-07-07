#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API MaterialPreviewSceneSource : public ISceneSource
    {
    public:
        MaterialPreviewSceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}