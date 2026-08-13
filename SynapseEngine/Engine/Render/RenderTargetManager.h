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
#include "Engine/Vk/Rendering/RenderTargetGroup.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace Syn {

    class SYN_API RenderTargetManager {
    public:
        explicit RenderTargetManager(uint32_t framesInFlight);
        ~RenderTargetManager() = default;

        RenderTargetManager(const RenderTargetManager&) = delete;
        RenderTargetManager& operator=(const RenderTargetManager&) = delete;

        void CreateGroup(const std::string& name);
        void AddAttachment(const std::string& groupName, const std::string& attachmentName, const Vk::ImageConfig& config);

        Vk::RenderTargetGroup* GetGroup(const std::string& name, uint32_t frameIndex) const;

        void Resize(uint32_t frameIndex, uint32_t width, uint32_t height);

        void RemoveGroup(const std::string& name);
    private:
        uint32_t _framesInFlight;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Vk::RenderTargetGroup>>> _groups;
    };
}