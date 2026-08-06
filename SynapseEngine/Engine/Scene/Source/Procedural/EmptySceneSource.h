#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"

namespace Syn
{
    class SYN_API EmptySceneSource : public ISceneSource
    {
    public:
        EmptySceneSource() = default;
        virtual bool Populate(Scene& scene) override;
    };
}