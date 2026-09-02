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
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Environment/Data/EnvironmentData.h"
#include "Engine/Environment/Uploader/IEnvironmentUploader.h"
#include <functional>

namespace Syn {
    class IImageSource;

    struct EnvironmentManagerCallbacks {
        std::function<void(uint32_t, VkImageView)> updateCubeTexture;
        std::function<uint32_t(const std::string&)> getSamplerIndex;
        std::function<uint32_t(const std::string&, std::function<std::unique_ptr<IImageSource>()>)> loadProceduralImage;
        std::function<uint32_t(const std::string&)> loadImageSync;
        std::function<uint32_t(const std::string&)> loadImageAsync;
        std::function<void(uint32_t)> waitForImage;
        std::function<std::shared_ptr<Texture>(uint32_t)> getImageResource;
    };

    class SYN_API EnvironmentManager : public AddressResourceManager<Environment, EnvironmentData> {
    public:
        EnvironmentManager(
            uint32_t framesInFlight,
            std::shared_ptr<ImageBuilder> imageBuilder,
            std::unique_ptr<IEnvironmentUploader> uploader,
            EnvironmentManagerCallbacks callbacks);

        ~EnvironmentManager() override = default;

        uint32_t LoadEnvironmentSync(const std::string& filePath);
        uint32_t LoadEnvironmentAsync(const std::string& filePath);

        uint32_t GetBrdfLutId() const { return _brdfLutId; }
    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
        std::shared_ptr<Environment> CreateEnvironmentResource(uint32_t skyTexId);
    private:
        EnvironmentManagerCallbacks _callbacks;
        std::shared_ptr<ImageBuilder> _imageBuilder;
        std::unique_ptr<IEnvironmentUploader> _uploader;
        uint32_t _brdfLutId = UINT32_MAX;
    };
}