#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API TestSceneSource : public ISceneSource
    {
    public:
        TestSceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}