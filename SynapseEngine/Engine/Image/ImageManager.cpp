#include "ImageManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Vk/Descriptor/DescriptorLayoutBuilder.h";
#include "Engine/Image/Source/Procedural/DefaultImageSource.h"
#include "SamplerNames.h"

namespace Syn {

    ImageManager::ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader)
        : _builder(builder), _uploader(std::move(uploader))
    {
        InitializeBindlessSetup();
    }

    void ImageManager::InitializeBindlessSetup()
    {
        Vk::DescriptorLayoutBuilder layoutBuilder;
        layoutBuilder.AddBindlessBinding(BINDING_SAMPLERS, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL, MAX_SAMPLERS);
        layoutBuilder.AddBindlessBinding(BINDING_TEXTURES, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, MAX_IMAGES);
        _bindlessLayout = layoutBuilder.Build(Vk::DescriptorLayoutType::DescriptorBuffer);
        _bindlessBuffer = std::make_unique<Vk::DescriptorBuffer>(_bindlessLayout);

        CreateSamplers();
        LoadDefaultImageSync();
    }

    void ImageManager::CreateSamplers() {
        {
            // Linear Repeat
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
            // Linear Repeat
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
            // Nearest Repeat
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
            // Nearest Clamp Edge
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
            // LinearAniso
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
            // NearestAniso
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
            // MaxReduction (HZB)
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
            // BloomSampler
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
            // ShadowSampler (PCF)
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
    }

    uint32_t ImageManager::RegisterSampler(const std::string& name, const Vk::SamplerConfig& config) {
        uint32_t index = static_cast<uint32_t>(_samplers.size());
        auto sampler = std::make_unique<Vk::Sampler>(config);

        _bindlessBuffer->WriteSampler(BINDING_SAMPLERS, index, sampler->Handle());

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
        uint32_t defaultId = InternalLoadSync("DefaultTexture", [this]() {
            auto source = std::make_unique<DefaultImageSource>();
            return _builder->BuildFromSource(*source);
            });

        auto entry = &_entries[defaultId];
        entry->resource = entry->cpuFuture.get();
        entry->state = ResourceState::UploadingGPU;

        ServiceLocator::GetGpuUploader()->UploadSync({
            .uploadCallback = [this, entry](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(entry->resource->gpuData, cmd);
                entry->resource->image = uploadResult.texture;
                entry->stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, entry]() {
                entry->stagingBuffer.reset();
                entry->state = ResourceState::Ready;
            },
            .needsGraphics = false
            });

        _bindlessBuffer->FillSampledImages(BINDING_TEXTURES, MAX_IMAGES, entry->resource->image->GetView());
        //Info("Default Fallback Texture (ID: 0) generated and bound to all bindless slots.");
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

    void ImageManager::StartGpuUpload(EntryType& entry) {
        bool needsGraphics = entry.resource->gpuData.autoGenerateMipmaps;

        ServiceLocator::GetGpuUploader()->Enqueue({
            .uploadCallback = [this, &entry](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(entry.resource->gpuData, cmd);
                entry.resource->image = uploadResult.texture;
                entry.stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, &entry]() {
                FinalizeResource(entry);
                entry.stagingBuffer.reset();
                entry.state = ResourceState::Ready;
                _version.fetch_add(1, std::memory_order_release);
				//Info("Image '{}' is ready", entry.path);
            },
            .needsGraphics = needsGraphics
            });
    }

    void ImageManager::FinalizeResource(EntryType& entry) {
        uint32_t descriptorIndex = _pathToId.at(entry.path);

        if (descriptorIndex != 0 && _bindlessBuffer) {
            _bindlessBuffer->WriteSampledImage(
                BINDING_TEXTURES,
                descriptorIndex,
                entry.resource->image->GetView()
            );
        }
    }
}