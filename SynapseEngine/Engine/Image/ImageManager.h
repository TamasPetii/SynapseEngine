#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Image/Uploader/IGpuImageUploader.h"
#include "Engine/Image/Converter/ICpuImageExtractor.h"  

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Descriptor/DescriptorBuffer.h"
#include "Engine/Vk/Image/Sampler.h"
#include "Engine/Vk/Descriptor/DescriptorPool.h"

#include "Engine/Manager/AddressResourceManager.h"

namespace Syn {

    using ImageSourceFactory = std::function<std::unique_ptr<IImageSource>()>;
    using ImageReadyOrChangedCallback = std::function<void(uint32_t imageId)>;

    class SYN_API ImageManager : public AddressResourceManager<Texture, uint32_t> {
    public:
        static constexpr uint32_t MAX_IMAGES = 2048;
        static constexpr uint32_t MAX_SAMPLERS = 32;
        static constexpr uint32_t BINDING_SAMPLERS = 0;
        static constexpr uint32_t BINDING_TEXTURES = 1;

        ImageManager(
            uint32_t framesInFlight,
            std::shared_ptr<ImageBuilder> builder,
            std::unique_ptr<IGpuImageUploader> uploader,
            std::unique_ptr<ICpuImageExtractor> cpuExtractor,
            ImageReadyOrChangedCallback imageReadyCallback = nullptr);

        ~ImageManager();

        void Update() override;
        void RecordSync(VkCommandBuffer cmd);

        uint32_t LoadImageAsync(const std::string& filePath);
        uint32_t LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory);

        uint32_t LoadImageSync(const std::string& filePath);
        uint32_t LoadImageFromSourceSync(const std::string& name, ImageSourceFactory factory);

        Vk::DescriptorBuffer* GetBindlessBuffer() const { return _bindlessBuffer.get(); }
        VkDescriptorSetLayout GetBindlessLayout() const { return _bindlessLayout; }
        Vk::Sampler* GetSampler(const std::string& name) const;
        uint32_t GetSamplerIndex(const std::string& name) const;

        const std::unordered_map<std::string, uint32_t>& GetAvailableSamplers() const { return _samplerNameToIndex; }
    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        void CreateSamplers();
        void InitializeBindlessSetup();
        void LoadDefaultImageSync();
        uint32_t RegisterSampler(const std::string& name, const Vk::SamplerConfig& config);
    private:
        ImageReadyOrChangedCallback _imageReadyOrChangedCallback;

        std::shared_ptr<ImageBuilder> _builder;
        std::unique_ptr<IGpuImageUploader> _uploader;
        std::unique_ptr<ICpuImageExtractor> _cpuExtractor;
       
        uint32_t _framesInFlight;
        std::mutex _staleMutex;
        std::vector<StaleBuffer> _staleGpuBuffers;
        std::vector<StaleBuffer> _staleMappedBuffers;

        VkDescriptorSetLayout _bindlessLayout = VK_NULL_HANDLE;
        std::unique_ptr<Vk::DescriptorBuffer> _bindlessBuffer;

        std::vector<std::unique_ptr<Vk::Sampler>> _samplers;
        std::unordered_map<std::string, uint32_t> _samplerNameToIndex;
    };
}