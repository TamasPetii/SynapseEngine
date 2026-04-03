#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API ILodProcessor : public IMeshProcessor
    {
    public:
        virtual ~ILodProcessor() override = default;
    };
}