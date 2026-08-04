#pragma once
#include "Engine/SynApi.h"
#include "../IAudioSource.h"
#include <optional>
#include <string>

namespace Syn
{
    class SYN_API ProceduralAudioSource : public IAudioSource
    {
    public:
        ProceduralAudioSource(std::string name) : _name(std::move(name)) {}
    protected:
        std::string _name;
    };
}