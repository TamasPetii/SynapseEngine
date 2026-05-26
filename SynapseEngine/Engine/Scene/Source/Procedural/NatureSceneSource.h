#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API NatureSceneSource : public ISceneSource
    {
    public:
        NatureSceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}