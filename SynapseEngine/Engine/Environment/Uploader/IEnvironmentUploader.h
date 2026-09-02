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
#include "Engine/Environment/Data/EnvironmentData.h"
#include "Engine/Vk/Rendering/GpuUploader.h"

namespace Syn {
    struct EnvironmentUploadResult {
        std::shared_ptr<Texture> baseCubemap;
        std::shared_ptr<Texture> irradianceMap;
        std::shared_ptr<Texture> prefilteredMap;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
    };

    class SYN_API IEnvironmentUploader {
    public:
        virtual ~IEnvironmentUploader() = default;
        virtual EnvironmentUploadResult Upload(Environment& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) = 0;
    };
}