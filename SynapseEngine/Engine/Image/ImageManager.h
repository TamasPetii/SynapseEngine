#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Image/Uploader/IGpuImageUploader.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Descriptor/DescriptorBuffer.h"
#include "Engine/Vk/Image/Sampler.h"

namespace Syn {

    using ImageSourceFactory = std::function<std::unique_ptr<IImageSource>()>;

    class SYN_API ImageManager : public BaseResourceManager<Texture> {
    public:
        static constexpr uint32_t MAX_IMAGES = 10000;
        static constexpr uint32_t MAX_SAMPLERS = 32;
        static constexpr uint32_t BINDING_SAMPLERS = 0;
        static constexpr uint32_t BINDING_TEXTURES = 1;

        ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader);
        ~ImageManager() = default;

        uint32_t LoadImageAsync(const std::string& filePath);
        uint32_t LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory);

        Vk::DescriptorBuffer* GetBindlessBuffer() const { return _bindlessBuffer.get(); }
        VkDescriptorSetLayout GetBindlessLayout() const { return _bindlessLayout; }
        Vk::Sampler* GetSampler(const std::string& name) const;
        uint32_t GetSamplerIndex(const std::string& name) const;
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        void CreateSamplers();
        void InitializeBindlessSetup();
        void LoadDefaultImageSync();
        uint32_t RegisterSampler(const std::string& name, const Vk::SamplerConfig& config);
    private:
        std::shared_ptr<ImageBuilder> _builder;
        std::unique_ptr<IGpuImageUploader> _uploader;

        VkDescriptorSetLayout _bindlessLayout = VK_NULL_HANDLE;
        std::unique_ptr<Vk::DescriptorBuffer> _bindlessBuffer;

        std::vector<std::unique_ptr<Vk::Sampler>> _samplers;
        std::unordered_map<std::string, uint32_t> _samplerNameToIndex;
    };
}