#include "ImageManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Logger/SynLog.h"
#include "SamplerNames.h"
#include "Engine/Vk/Descriptor/DescriptorWriter.h"
#include "ImageNames.h"
#include "Engine/Image/Source/Procedural/DefaultImageSource.h"
#include "Engine/Vk/Descriptor/DescriptorLayoutBuilder.h";

namespace Syn 
{
    ImageManager::ImageManager(
        uint32_t framesInFlight,
        std::shared_ptr<ImageBuilder> builder,
        std::unique_ptr<IGpuImageUploader> uploader,
        std::unique_ptr<ICpuImageExtractor> cpuExtractor,
        ImageReadyOrChangedCallback imageReadyCallback)
		: AddressResourceManager<Texture, uint32_t>(framesInFlight, 1024, 256, 512),
		_framesInFlight(framesInFlight),
        _builder(builder), 
        _uploader(std::move(uploader)), 
        _cpuExtractor(std::move(cpuExtractor)),
        _imageReadyOrChangedCallback(std::move(imageReadyCallback))
    {
        InitializeBindlessSetup();
    }

    ImageManager::~ImageManager() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();

        if (_bindlessLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, _bindlessLayout, nullptr);
            _bindlessLayout = VK_NULL_HANDLE;
        }
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

    void ImageManager::Update() {
        BaseResourceManager<Texture>::Update();

        std::lock_guard<std::mutex> lock(_staleMutex);

        for (auto it = _staleGpuBuffers.begin(); it != _staleGpuBuffers.end();) {
            if (it->framesToLive > 0) {
                it->framesToLive--;
                ++it;
            }
            else {
                it = _staleGpuBuffers.erase(it);
            }
        }

        for (auto it = _staleMappedBuffers.begin(); it != _staleMappedBuffers.end();) {
            if (it->framesToLive > 0) {
                it->framesToLive--;
                ++it;
            }
            else {
                it = _staleMappedBuffers.erase(it);
            }
        }
    }

    void ImageManager::RecordSync(VkCommandBuffer cmd) {
        if (auto staleBuffers = _bindlessBuffer->RecordSync(cmd); staleBuffers.mapped || staleBuffers.gpu) {
            std::lock_guard<std::mutex> lock(_staleMutex);

            _staleMappedBuffers.push_back({ staleBuffers.mapped, _framesInFlight });
            _staleGpuBuffers.push_back({ staleBuffers.gpu, _framesInFlight });
        }
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
            // Linear Clamp To Edge
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
            // Nearest Clamp To Edge
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
            RegisterSampler(SamplerNames::SkyboxSampler, config);
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
        uint32_t defaultId = InternalLoadSync(ImageNames::Default, [this]() {
            auto source = std::make_unique<DefaultImageSource>();
            return _builder->BuildFromSource(*source);
            });

        auto resource = GetResource(defaultId);

        if (resource && resource->image) {
            _bindlessBuffer->FillSampledImages(BINDING_TEXTURES, MAX_IMAGES, resource->image->GetView());
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
            .uploadCallback = [this, entryId, res](VkCommandBuffer cmd) {

                auto uploadResult = _uploader->Upload(*(res->transientGpuData), cmd); 
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
            .needsGraphics = needsGraphics
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
        //Just to handle bistro test, this will be deleted!!
        auto isNormalMap = [](const std::string& path) {
            std::string lowerPath = path;
            std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);
            return lowerPath.find("_normal") != std::string::npos;
            };

        ProcessDirtyReadyEntries(
            [this, &isNormalMap](uint32_t index, const EntryType& entry) {
                if (!entry.resource->image) return;

                _bindlessBuffer->WriteSampledImage(
                    BINDING_TEXTURES,
                    index,
                    entry.resource->image->GetView()
                );

                uint32_t samplerIndex = GetSamplerIndex("LinearAniso");
                bool invertTangent = isNormalMap(entry.path);

                uint32_t textureData = (samplerIndex & 0x7FFFFFFF);
                if (invertTangent) {
                    textureData |= (1u << 31);
                }

                WriteAddress(index, textureData);

                if (_imageReadyOrChangedCallback) {
                    _imageReadyOrChangedCallback(index);
                }
            }
        );
    }
}