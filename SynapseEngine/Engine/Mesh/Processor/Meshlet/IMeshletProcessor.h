#pragma once
#include "Engine/SynApi.h"
#include "../IMeshProcessor.h"

namespace Syn
{
    class SYN_API IMeshletProcessor : public IMeshProcessor
    {
    public:
        virtual ~IMeshletProcessor() override = default;
    };
}