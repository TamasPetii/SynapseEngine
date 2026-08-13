// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "Sampler.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {

    Sampler::Sampler(const SamplerConfig& config)
        : _config(config) {

        auto context = ServiceLocator::Get<Vk::Context>();
        auto device = context->GetDevice();
        auto physicalDevice = context->GetPhysicalDevice();

        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = _config.magFilter;
        samplerInfo.minFilter = _config.minFilter;
        samplerInfo.addressModeU = _config.addressModeU;
        samplerInfo.addressModeV = _config.addressModeV;
        samplerInfo.addressModeW = _config.addressModeW;
        samplerInfo.anisotropyEnable = _config.anisotropyEnable ? VK_TRUE : VK_FALSE;

        if (_config.anisotropyEnable) {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice->Handle(), &properties);
            samplerInfo.maxAnisotropy = std::min(_config.maxAnisotropy, properties.limits.maxSamplerAnisotropy);
        }
        else {
            samplerInfo.maxAnisotropy = 1.0f;
        }

        samplerInfo.borderColor = _config.borderColor;
        samplerInfo.unnormalizedCoordinates = _config.unnormalizedCoordinates;
        samplerInfo.compareEnable = _config.compareEnable;
        samplerInfo.compareOp = _config.compareOp;
        samplerInfo.mipmapMode = _config.mipmapMode;
        samplerInfo.mipLodBias = _config.mipLodBias;
        samplerInfo.minLod = _config.minLod;
        samplerInfo.maxLod = _config.maxLod;

        VkSamplerReductionModeCreateInfoEXT reductionInfo{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT };
        if (_config.reductionMode.has_value()) {
            reductionInfo.reductionMode = _config.reductionMode.value();
            samplerInfo.pNext = &reductionInfo;
        }

        SYN_VK_ASSERT_MSG(vkCreateSampler(device->Handle(), &samplerInfo, nullptr, &_handle), "Failed to create Sampler");
    }

    Sampler::~Sampler() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
            vkDestroySampler(device->Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

}