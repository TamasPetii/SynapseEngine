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

#pragma once
#include "Image.h"

namespace Syn::Vk {

    class SYN_API ImageFactory {
    public:
        static void Allocate(Image* image);
        static void CreateViews(Image* image);
        static std::unique_ptr<Image> Create(const ImageConfig& config);
        static std::unique_ptr<Image> CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, bool mipmaps = true);
        static std::unique_ptr<Image> CreateAttachment(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
        static std::unique_ptr<Image> CreateCubemap(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
    };
}