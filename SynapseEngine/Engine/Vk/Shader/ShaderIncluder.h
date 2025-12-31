#pragma once
#include "Engine/SynApi.h"
#include <shaderc/shaderc.hpp>
#include <string>
#include <vector>

namespace Syn::Vk {
    class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
    public:
        shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override;
        void ReleaseInclude(shaderc_include_result* data) override;
    };
}