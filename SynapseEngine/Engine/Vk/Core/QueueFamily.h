#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics;
        std::optional<uint32_t> compute;
        std::optional<uint32_t> transfer;
        std::optional<uint32_t> present;

        bool IsComplete() const {
            return 
                graphics.has_value() && 
                compute.has_value() && 
                transfer.has_value() && 
                present.has_value();
        }
    };
}

