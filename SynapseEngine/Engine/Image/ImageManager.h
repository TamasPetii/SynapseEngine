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
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Image/Uploader/IGpuImageUploader.h"
#include "Engine/Image/Converter/ICpuImageExtractor.h"  
#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Image/Sampler.h"
#include "Engine/Manager/AddressResourceManager.h"
#include <functional>

namespace Syn {
    using ImageSourceFactory = std::function<std::unique_ptr<IImageSource>()>;

    struct ImageManagerCallbacks {
        std::function<void(uint32_t, VkSampler)> registerSampler;
        std::function<void(VkImageView)> fillDefaultTexture;
        std::function<void(uint32_t, VkImageView)> updateTexture;
        std::function<void(uint32_t)> notifyMaterialManager;
    };

    class SYN_API ImageManager : public AddressResourceManager<Texture, uint32_t> {
    public:
        ImageManager(
            uint32_t framesInFlight,
            std::shared_ptr<ImageBuilder> builder,
            std::unique_ptr<IGpuImageUploader> uploader,
            std::unique_ptr<ICpuImageExtractor> cpuExtractor,
            ImageManagerCallbacks callbacks);

        ~ImageManager() override = default;

        uint32_t LoadImageAsync(const std::string& filePath);
        uint32_t LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory);
        uint32_t LoadImageSync(const std::string& filePath);
        uint32_t LoadImageFromSourceSync(const std::string& name, ImageSourceFactory factory);

        Vk::Sampler* GetSampler(const std::string& name) const;
        uint32_t GetSamplerIndex(const std::string& name) const;
        const std::unordered_map<std::string, uint32_t>& GetAvailableSamplers() const { return _samplerNameToIndex; }

    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        void CreateSamplers();
        void LoadDefaultImageSync();
        uint32_t RegisterSampler(const std::string& name, const Vk::SamplerConfig& config);
    private:
        ImageManagerCallbacks _callbacks;
        std::shared_ptr<ImageBuilder> _builder;
        std::unique_ptr<IGpuImageUploader> _uploader;
        std::unique_ptr<ICpuImageExtractor> _cpuExtractor;

        std::vector<std::unique_ptr<Vk::Sampler>> _samplers;
        std::unordered_map<std::string, uint32_t> _samplerNameToIndex;
    };
}