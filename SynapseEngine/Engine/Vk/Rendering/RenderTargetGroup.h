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
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace Syn::Vk {

    class SYN_API RenderTargetGroup {
    public:
        RenderTargetGroup() = default;
        ~RenderTargetGroup() = default;

        RenderTargetGroup(const RenderTargetGroup&) = delete;
        RenderTargetGroup& operator=(const RenderTargetGroup&) = delete;

        void AddAttachment(const std::string& name, const ImageConfig& config);

        void Resize(uint32_t width, uint32_t height);

        Image* GetImage(const std::string& name) const;

        uint32_t GetWidth() const { return _width; }
        uint32_t GetHeight() const { return _height; }
    private:
        void CreateOrRecreateImages();
    private:
        std::unordered_map<std::string, ImageConfig> _configs;
        std::unordered_map<std::string, std::unique_ptr<Image>> _images;

        uint32_t _width = 0;
        uint32_t _height = 0;
    };
}