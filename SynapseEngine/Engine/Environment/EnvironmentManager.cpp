#include "EnvironmentManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Image/Source/Procedural/BrdfLutImageSource.h"

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
    {
        if (_callbacks.loadProceduralImage) {
            _brdfLutId = _callbacks.loadProceduralImage("BrdfLut", []() {
                return std::make_unique<BrdfLutImageSource>(512, 1024);
                });
        }
    }

    std::shared_ptr<Environment> EnvironmentManager::CreateEnvironmentResource(uint32_t skyTexId) {
        if (_callbacks.waitForImage && skyTexId != UINT32_MAX) {
            _callbacks.waitForImage(skyTexId);
        }

        auto env = std::make_shared<Environment>();
        env->skyTextureId = skyTexId;

        if (_callbacks.getImageResource && skyTexId != UINT32_MAX) {
            env->transientSourceImage = _callbacks.getImageResource(skyTexId);
        }

        return env;
    }

    uint32_t EnvironmentManager::LoadEnvironmentAsync(const std::string& filePath) {
        uint32_t skyTexId = UINT32_MAX;

        if (_callbacks.loadImageAsync) {
            skyTexId = _callbacks.loadImageAsync(filePath);
        }

        return this->InternalLoadAsync(filePath, [this, skyTexId]() {
            return CreateEnvironmentResource(skyTexId);
            });
    }

    uint32_t EnvironmentManager::LoadEnvironmentSync(const std::string& filePath) {
        uint32_t skyTexId = UINT32_MAX;

        if (_callbacks.loadImageSync) {
            skyTexId = _callbacks.loadImageSync(filePath);
        }

        return this->InternalLoadSync(filePath, [this, skyTexId]() {
            return CreateEnvironmentResource(skyTexId);
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

                glm::vec3 rotRads = glm::radians(entry.resource->skyRotation);
                glm::mat4 rotMat = glm::mat4(1.0f);
                rotMat = glm::rotate(rotMat, rotRads.y, glm::vec3(0.0f, 1.0f, 0.0f));
                rotMat = glm::rotate(rotMat, rotRads.x, glm::vec3(1.0f, 0.0f, 0.0f));
                rotMat = glm::rotate(rotMat, rotRads.z, glm::vec3(0.0f, 0.0f, 1.0f));

                EnvironmentData data{};
                data.skyRotationMatrix = rotMat;
                data.skyTint = entry.resource->skyTint;
                data.skyExposureEV = entry.resource->skyExposureEV;
                data.intensity = entry.resource->intensity;
                data.ambientIntensity = entry.resource->ambientIntensity;
                data.skyTextureIndex = entry.resource->skyTextureId;

                uint32_t cubeSampler = 0;
                uint32_t sphereSampler = 0;

                if (_callbacks.getSamplerIndex) {
                    cubeSampler = _callbacks.getSamplerIndex(entry.resource->cubemapSamplerName);
                    sphereSampler = _callbacks.getSamplerIndex(entry.resource->skySphereSamplerName);
                }

                data.packedSamplers = (sphereSampler << 16) | (cubeSampler & 0xFFFF);

                addressUpdates.push_back({ index, data });
            }
        );

        if (!addressUpdates.empty()) {
            this->WriteAddresses(addressUpdates);
        }
    }
}