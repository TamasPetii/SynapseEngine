#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    class Scene;

    class SYN_API ISceneSource
    {
    public:
        virtual ~ISceneSource() = default;
        virtual bool Populate(Scene& scene) = 0;
    };
}