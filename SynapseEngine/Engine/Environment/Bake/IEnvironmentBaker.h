#pragma once
#include "Engine/SynApi.h"
#include "EnvironmentBakeContext.h"

namespace Syn 
{
    class SYN_API IEnvironmentBaker {
    public:
        virtual ~IEnvironmentBaker() = default;
        virtual void Initialize() = 0;
        virtual void Bake(const EnvironmentBakeContext& context) = 0;
    };
}