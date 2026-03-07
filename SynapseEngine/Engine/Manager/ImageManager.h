#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Image/Uploader/IGpuImageUploader.h"

#include <vector>
#include <memory>
#include <string>
#include <future>
#include <unordered_map>

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Command/CommandBuffer.h"
#include "Engine/Vk/Synchronization/Fence.h"

namespace Syn
{
    using ImageSourceFactory = std::function<std::unique_ptr<IImageSource>()>;

    enum class SYN_API ImageState {
        LoadingCPU,
        UploadingGPU,
        Ready,
        Failed
    };

    struct SYN_API ImageEntry {
        ImageState state;
        std::string path;

        std::shared_ptr<Texture> texture;

        std::unique_ptr<Vk::Fence> uploadFence;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        std::unique_ptr<Vk::CommandBuffer> transferCmd;

        Vk::ThreadSafeQueue* usedQueue = nullptr;
        std::future<std::optional<GpuImage>> cpuFuture;
    };

    class SYN_API ImageManager
    {
    public:
        ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader);
        ~ImageManager() = default;

        uint32_t LoadImageAsync(const std::string& filePath);
        uint32_t LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory);

        void Update();

        std::shared_ptr<Texture> GetTexture(uint32_t id) const;
        std::shared_ptr<Texture> GetTexture(const std::string& name) const;
    private:
        using ImageLoadTask = std::function<std::optional<GpuImage>()>;
        uint32_t InternalLoadAsync(const std::string& key, ImageLoadTask task);
    private:
        std::shared_ptr<ImageBuilder> _builder;
        std::unique_ptr<IGpuImageUploader> _uploader;

        Vk::ThreadSafeQueue* _transferQueue = nullptr;
        Vk::ThreadSafeQueue* _graphicsQueue = nullptr;

        std::unique_ptr<Vk::CommandPool> _transferPool;
        std::unique_ptr<Vk::CommandPool> _graphicsPool;

        std::vector<ImageEntry> _images;
        std::unordered_map<std::string, uint32_t> _pathToId;
    };
}