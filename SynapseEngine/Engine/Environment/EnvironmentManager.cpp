#include "EnvironmentManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn {
    EnvironmentManager::EnvironmentManager(
        uint32_t framesInFlight,
        std::shared_ptr<ImageBuilder> imageBuilder,
        std::unique_ptr<IEnvironmentUploader> uploader,
        EnvironmentManagerCallbacks callbacks)
        : AddressResourceManager<Environment, EnvironmentData>(framesInFlight, 64, 16, 32),
        _callbacks(std::move(callbacks)),
        _imageBuilder(std::move(imageBuilder)),
        _uploader(std::move(uploader))
    {}

    uint32_t EnvironmentManager::LoadEnvironmentAsync(const std::string& filePath) {
        return this->InternalLoadAsync(filePath, [this, filePath]() {
            auto env = std::make_shared<Environment>();
            env->transientSourceImage = _imageBuilder->BuildFromFile(filePath);
            return env;
            });
    }

    void EnvironmentManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryId = this->_pathToId.at(entry.path);
        std::shared_ptr<Environment> res = entry.resource;

        Vk::GpuUploadRequest request{
            .uploadCallback = [this, entryId, res](VkCommandBuffer cmd, Vk::GpuUploader* gpuUploader) {
                if (_uploader) {
                    auto uploadResult = _uploader->Upload(*res, cmd, gpuUploader);
                    res->baseCubemap = uploadResult.baseCubemap;
                    res->irradianceMap = uploadResult.irradianceMap;
                    res->prefilteredMap = uploadResult.prefilteredMap;

                    std::lock_guard lock(_mutex);
                    _entries[entryId].stagingBuffer = std::move(uploadResult.stagingBuffer);
                }
            },
            .onFinished = [this, entryId]() {
                std::lock_guard lock(_mutex);
                auto& entry = _entries[entryId];

                FinalizeResource(entry);
                entry.stagingBuffer.reset();

                SetResourceState(entryId, ResourceState::Ready);
                MarkDirty(entryId);
            },
            .queueType = Vk::GpuQueueType::Compute
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void EnvironmentManager::FinalizeResource(EntryType& entry) {
        entry.resource->transientSourceImage.reset();
    }

    void EnvironmentManager::FlushDirtyResources() {
        std::vector<std::pair<uint32_t, EnvironmentData>> addressUpdates;

        this->ProcessDirtyReadyEntries(
            [this, &addressUpdates](uint32_t index, const EntryType& entry) {
                if (!entry.resource || !entry.resource->baseCubemap || !entry.resource->baseCubemap->image) return;

                uint32_t baseCubeIdx = index * 3 + 0;
                uint32_t irradianceIdx = index * 3 + 1;
                uint32_t prefilteredIdx = index * 3 + 2;

                if (_callbacks.updateCubeTexture) {
                    _callbacks.updateCubeTexture(baseCubeIdx, entry.resource->baseCubemap->image->GetView());
                    _callbacks.updateCubeTexture(irradianceIdx, entry.resource->irradianceMap->image->GetView());
                    _callbacks.updateCubeTexture(prefilteredIdx, entry.resource->prefilteredMap->image->GetView());
                }

                EnvironmentData data{};
                data.intensity = entry.resource->intensity;
                data.skyTint[0] = entry.resource->skyTint.x;
                data.skyTint[1] = entry.resource->skyTint.y;
                data.skyTint[2] = entry.resource->skyTint.z;

                data.baseCubemapIndex = baseCubeIdx;
                data.irradianceCubeIndex = irradianceIdx;
                data.prefilteredCubeIndex = prefilteredIdx;
                data.brdfLutTexIndex = UINT32_MAX;

                addressUpdates.push_back({ index, data });
            }
        );

        if (!addressUpdates.empty()) {
            this->WriteAddresses(addressUpdates);
        }
    }
}