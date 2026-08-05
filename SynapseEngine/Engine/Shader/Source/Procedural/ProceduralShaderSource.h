#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Source/IShaderSource.h"
#include <string>

namespace Syn {
    class SYN_API ProceduralShaderSource : public IShaderSource {
    public:
        ProceduralShaderSource(std::string name);
    protected:
        std::string _name;
    };
}