#include "ImageManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    ImageManager::ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader)
        : _builder(builder),
        _uploader(std::move(uploader))
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        _transferQueue = device->GetTransferQueue();
        _graphicsQueue = device->GetGraphicsQueue();

        if (!_transferQueue) {
            _transferQueue = _graphicsQueue;
        }

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        if (_transferQueue != _graphicsQueue) {
            _graphicsPool = std::make_unique<Vk::CommandPool>(_graphicsQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        }
    }

    uint32_t ImageManager::InternalLoadAsync(const std::string& key, ImageLoadTask task)
    {
        if (_pathToId.contains(key)) {
            return _pathToId[key];
        }

        uint32_t newId = static_cast<uint32_t>(_images.size());
        _pathToId[key] = newId;

        ImageEntry entry;
        entry.path = key;
        entry.state = ImageState::LoadingCPU;

        auto executor = ServiceLocator::GetTaskExecutor();
        entry.cpuFuture = executor->async(std::move(task));

        _images.push_back(std::move(entry));
        return newId;
    }

    uint32_t ImageManager::LoadImageAsync(const std::string& filePath) {
        return InternalLoadAsync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t ImageManager::LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory) {
        return InternalLoadAsync(name, [this, factory]() -> std::optional<GpuImage> {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::nullopt;
            });
    }

    void ImageManager::Update()
    {
        for (auto& entry : _images)
        {
            if (entry.state == ImageState::LoadingCPU)
            {
                if (entry.cpuFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    auto gpuDataOpt = entry.cpuFuture.get();

                    if (!gpuDataOpt) {
                        Error("Failed to load image: {}", entry.path);
                        entry.state = ImageState::Failed;
                        continue;
                    }

                    auto& gpuData = gpuDataOpt.value();

                    bool needsGraphics = gpuData.autoGenerateMipmaps;
                    auto& selectedPool = (needsGraphics && _graphicsPool) ? _graphicsPool : _transferPool;
                    entry.usedQueue = needsGraphics ? _graphicsQueue : _transferQueue;

                    entry.transferCmd = selectedPool->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
                    entry.uploadFence = std::make_unique<Vk::Fence>(false);
                    entry.transferCmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

                    auto uploadResult = _uploader->Upload(gpuData, entry.transferCmd->Handle());

                    entry.transferCmd->End();
                    entry.texture = uploadResult.texture;
                    entry.stagingBuffer = std::move(uploadResult.stagingBuffer);

                    VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
                    cmdSubmitInfo.commandBuffer = entry.transferCmd->Handle();

                    VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
                    submitInfo.commandBufferInfoCount = 1;
                    submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

                    entry.usedQueue->Submit(&submitInfo, entry.uploadFence->Handle());

                    Info("Image queued for GPU upload: {} (Queue: {})", entry.path, needsGraphics ? "Graphics" : "Transfer");
                    entry.state = ImageState::UploadingGPU;
                }
            }
            else if (entry.state == ImageState::UploadingGPU)
            {
                if (entry.uploadFence->IsSignaled())
                {
                    Info("Image fully uploaded and ready: {}", entry.path);

                    entry.stagingBuffer.reset();
                    entry.transferCmd.reset();
                    entry.uploadFence.reset();

                    entry.state = ImageState::Ready;

                    //Todo: Descriptor
                }
            }
        }
    }

    std::shared_ptr<Texture> ImageManager::GetTexture(uint32_t id) const
    {
        if (id >= _images.size()) return nullptr;
        if (_images[id].state == ImageState::Ready) return _images[id].texture;
        return nullptr;
    }

    std::shared_ptr<Texture> ImageManager::GetTexture(const std::string& name) const
    {
        if (_pathToId.find(name) == _pathToId.end()) return nullptr;
        return GetTexture(_pathToId.at(name));
    }
}