#include "ImageManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    ImageManager::ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader)
        : _builder(builder), _uploader(std::move(uploader))
    {
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
				Info("Image '{}' is ready", entry.path);
                _version++;
            },
            .needsGraphics = needsGraphics
            });
    }

    void ImageManager::FinalizeResource(EntryType& entry) {
        //auto globalResources = ServiceLocator::GetGlobalRenderResources();
        //uint32_t descriptorIndex = _pathToId[entry.path];
        //globalResources->GetBindlessBuffer()->WriteSampledImage(1, descriptorIndex, entry.resource->image->GetView());
    }
}