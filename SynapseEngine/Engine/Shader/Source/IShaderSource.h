#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Data/Raw/RawShader.h"
#include <optional>

namespace Syn {
    class SYN_API IShaderSource {
    public:
        virtual ~IShaderSource() = default;
        virtual std::optional<RawShader> Produce() = 0;
    };
}