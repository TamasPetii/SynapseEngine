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

#include "ImageManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Logger/SynLog.h"
#include "SamplerNames.h"
#include "ImageNames.h"
#include "Engine/Image/Source/Procedural/DefaultImageSource.h"

namespace Syn
{
    ImageManager::ImageManager(
        uint32_t framesInFlight,
        std::shared_ptr<ImageBuilder> builder,
        std::unique_ptr<IGpuImageUploader> uploader,
        std::unique_ptr<ICpuImageExtractor> cpuExtractor,
        ImageManagerCallbacks callbacks)
        : AddressResourceManager<Texture, uint32_t>(framesInFlight, 1024, 256, 512),
        _callbacks(std::move(callbacks)),
        _builder(builder),
        _uploader(std::move(uploader)),
        _cpuExtractor(std::move(cpuExtractor))
    {
        CreateSamplers();
        LoadDefaultImageSync();
    }

    void ImageManager::CreateSamplers() {
        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = false;
            RegisterSampler(SamplerNames::LinearRepeat, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = false;
            RegisterSampler(SamplerNames::LinearClampEdge, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_NEAREST;
            config.minFilter = VK_FILTER_NEAREST;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            config.anisotropyEnable = false;
            RegisterSampler(SamplerNames::NearestRepeat, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_NEAREST;
            config.minFilter = VK_FILTER_NEAREST;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            config.anisotropyEnable = false;
            RegisterSampler(SamplerNames::NearestClampEdge, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = true;
            RegisterSampler(SamplerNames::LinearAniso, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_NEAREST;
            config.minFilter = VK_FILTER_NEAREST;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            config.anisotropyEnable = true;
            RegisterSampler(SamplerNames::NearestAniso, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            config.anisotropyEnable = false;
            config.minLod = 0.0f;
            config.maxLod = 16.0f;
            config.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
            RegisterSampler(SamplerNames::MaxReduction, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = false;
            RegisterSampler(SamplerNames::BloomSampler, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            config.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            config.anisotropyEnable = false;
            config.compareEnable = true;
            config.compareOp = VK_COMPARE_OP_LESS;
            RegisterSampler(SamplerNames::ShadowSampler, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = true;
            RegisterSampler(SamplerNames::SkySphereSampler, config);
        }

        {
            Vk::SamplerConfig config{};
            config.magFilter = VK_FILTER_LINEAR;
            config.minFilter = VK_FILTER_LINEAR;
            config.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            config.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            config.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            config.anisotropyEnable = true;
            RegisterSampler(SamplerNames::SkyBoxSampler, config);
        }
    }

    uint32_t ImageManager::RegisterSampler(const std::string& name, const Vk::SamplerConfig& config) {
        uint32_t index = static_cast<uint32_t>(_samplers.size());
        auto sampler = std::make_unique<Vk::Sampler>(config);

        if (_callbacks.registerSampler) {
            _callbacks.registerSampler(index, sampler->Handle());
        }

        _samplers.push_back(std::move(sampler));
        _samplerNameToIndex[name] = index;
        return index;
    }

    uint32_t ImageManager::GetSamplerIndex(const std::string& name) const {
        auto it = _samplerNameToIndex.find(name);
        return it != _samplerNameToIndex.end() ? it->second : 0;
    }

    Vk::Sampler* ImageManager::GetSampler(const std::string& name) const
    {
        auto index = GetSamplerIndex(name);
        return _samplers[index].get();
    }

    void ImageManager::LoadDefaultImageSync() {
        uint32_t defaultId = InternalLoadSync(ImageNames::Default, [this]() {
            auto source = std::make_unique<DefaultImageSource>();
            return _builder->BuildFromSource(*source);
            });

        auto resource = GetResource(defaultId);

        if (resource && resource->image && _callbacks.fillDefaultTexture) {
            _callbacks.fillDefaultTexture(resource->image->GetView());
        }
    }

    uint32_t ImageManager::LoadImageAsync(const std::string& filePath) {
        return InternalLoadAsync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t ImageManager::LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory) {
        return InternalLoadAsync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<Texture>(nullptr);
            });
    }

    uint32_t ImageManager::LoadImageSync(const std::string& filePath) {
        return InternalLoadSync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t ImageManager::LoadImageFromSourceSync(const std::string& name, ImageSourceFactory factory) {
        return InternalLoadSync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<Texture>(nullptr);
            });
    }

    void ImageManager::StartGpuUpload(EntryType& entry) {
        bool needsGraphics = entry.resource->transientGpuData->autoGenerateMipmaps;

        uint32_t entryId = _pathToId.at(entry.path);
        std::shared_ptr<Texture> res = entry.resource;

        Vk::GpuUploadRequest request{
            .uploadCallback = [this, entryId, res](VkCommandBuffer cmd, Vk::GpuUploader* gpuUploader) {
                auto uploadResult = _uploader->Upload(*(res->transientGpuData), cmd, gpuUploader);
                res->image = uploadResult.texture;
                std::lock_guard lock(_mutex);
                _entries[entryId].stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, entryId]() {
                std::lock_guard lock(_mutex);
                auto& entry = _entries[entryId];
                FinalizeResource(entry);
                entry.stagingBuffer.reset();
                SetResourceState(entryId, ResourceState::Ready);
                MarkDirty(entryId);
                Info("Image '{}' is ready", entry.path);
            },
            .queueType = needsGraphics ? Vk::GpuQueueType::Graphics : Vk::GpuQueueType::Transfer
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void ImageManager::FinalizeResource(EntryType& entry)
    {
        _cpuExtractor->Extract(*(entry.resource->transientGpuData), entry.resource->cpuData);

        entry.resource->transientCpuData.reset();
        entry.resource->transientGpuData.reset();
    }

    void ImageManager::FlushDirtyResources() {
        auto isNormalMap = [](const std::string& path) {
            std::string lowerPath = path;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
            return lowerPath.find("_normal") != std::string::npos;
            };

        ProcessDirtyReadyEntries(
            [this, &isNormalMap](uint32_t index, const EntryType& entry) {
                if (!entry.resource->image) return;

                if (_callbacks.updateTexture) {
                    _callbacks.updateTexture(index, entry.resource->image->GetView());
                }

                uint32_t samplerIndex = GetSamplerIndex("LinearAniso");
                bool invertTangent = isNormalMap(entry.path);

                uint32_t textureData = (samplerIndex & 0x7FFFFFFF);
                if (invertTangent) {
                    textureData |= (1u << 31);
                }

                WriteAddress(index, textureData);

                if (_callbacks.notifyMaterialManager) {
                    _callbacks.notifyMaterialManager(index);
                }
            }
        );
    }
}