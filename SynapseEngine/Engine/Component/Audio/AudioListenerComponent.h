#pragma once
#include "Engine/SynApi.h"
#include "Engine/Component/Core/Component.h"

namespace Syn
{
    struct SYN_API AudioListenerComponent : public Component
    {
        AudioListenerComponent() = default;

        bool active = true;
    };
}