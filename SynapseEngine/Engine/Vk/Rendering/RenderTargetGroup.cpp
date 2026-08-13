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

#include "RenderTargetGroup.h"
#include "Engine/Logger/SynLog.h"

namespace Syn::Vk {

    void RenderTargetGroup::AddAttachment(const std::string& name, const ImageConfig& config) {
        _configs[name] = config;

        if (_width == 0 || _height == 0) {
            _width = config.width;
            _height = config.height;
        }

        _images[name] = std::make_unique<Image>(config);
    }

    void RenderTargetGroup::Resize(uint32_t width, uint32_t height) {
        if (_width == width && _height == height) {
            return;
        }

        _width = width;
        _height = height;

        for (auto& [name, config] : _configs) {
            config.width = width;
            config.height = height;
        }

        CreateOrRecreateImages();
    }

    Image* RenderTargetGroup::GetImage(const std::string& name) const {
        auto it = _images.find(name);
        if (it != _images.end()) {
            return it->second.get();
        }

        SYN_ASSERT(false, ("RenderTargetGroup: Image not found - " + name).c_str());
        return nullptr;
    }

    void RenderTargetGroup::CreateOrRecreateImages() {
        _images.clear();

        for (const auto& [name, config] : _configs) {
            _images[name] = std::make_unique<Image>(config);
        }
    }
}