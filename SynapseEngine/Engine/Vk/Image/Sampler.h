#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {

    struct SYN_API SamplerConfig {
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkFilter minFilter = VK_FILTER_LINEAR;

        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        bool anisotropyEnable = false;
        float maxAnisotropy = 16.0f;

        VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        bool unnormalizedCoordinates = false;

        bool compareEnable = false;
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;

        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = VK_LOD_CLAMP_NONE;

        std::optional<VkSamplerReductionMode> reductionMode = std::nullopt;
    };

    class SYN_API Sampler {
    public:
        explicit Sampler(const SamplerConfig& config);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&&) = delete;
        Sampler& operator=(Sampler&&) = delete;

        VkSampler Handle() const { return _handle; }
        const SamplerConfig& GetConfig() const { return _config; }
    private:
        SamplerConfig _config;
        VkSampler _handle = VK_NULL_HANDLE;
    };
}