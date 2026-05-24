#include "AnimationManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    AnimationManager::AnimationManager(
        uint32_t framesInFlight,
        std::shared_ptr<AnimationBuilder> builder,
        std::unique_ptr<IGpuAnimationUploader> uploader,
        std::unique_ptr<ICpuAnimationExtractor> cpuExtractor)
        : AddressResourceManager<Animation, GpuAnimationAddresses>(framesInFlight, 100, 256, 512),
        _builder(builder), 
        _uploader(std::move(uploader)), 
        _cpuExtractor(std::move(cpuExtractor))
    {
    }

    uint32_t AnimationManager::LoadAnimationAsync(const std::string& filePath, uint32_t baseModelId) {
        std::string uniqueName = filePath + "_" + std::to_string(baseModelId);

        return InternalLoadAsync(uniqueName, [this, filePath, baseModelId]() {
            auto modelManager = ServiceLocator::GetModelManager();

            modelManager->WaitForResource(baseModelId);

            if (modelManager->GetEntryState(baseModelId) == ResourceState::Failed) {
                Error("Animation cannot be created: Base model failed to load.");
                return std::shared_ptr<Animation>(nullptr);
            }

            auto baseModel = modelManager->GetResource(baseModelId);

            if (!baseModel) 
                return std::shared_ptr<Animation>(nullptr);

            return _builder->BuildFromFile(filePath, *baseModel->transientCpuData);
            });
    }

    uint32_t AnimationManager::LoadAnimationSync(const std::string& filePath, uint32_t baseModelId) {
        std::string uniqueName = filePath + "_" + std::to_string(baseModelId);

        return InternalLoadSync(uniqueName, [this, filePath, baseModelId]() {
            auto modelManager = ServiceLocator::GetModelManager();
            auto baseModel = modelManager->GetResource(baseModelId);

            if (!baseModel) 
                return std::shared_ptr<Animation>(nullptr);

            return _builder->BuildFromFile(filePath, *baseModel->transientCpuData);
            });
    }

    void AnimationManager::StartGpuUpload(EntryType& entry) {
        Vk::GpuUploadRequest request{
            .uploadCallback = [this, &entry](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(*(entry.resource->transientGpuData), cmd);
                entry.resource->hardwareBuffers = std::move(uploadResult.hardwareBuffers);
                entry.stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, &entry]() {
                FinalizeResource(entry);
                entry.stagingBuffer.reset();
                entry.state = ResourceState::Ready;
                _version.fetch_add(1, std::memory_order_release);
                Info("Animation loaded, extracted, and RAM freed: {}", entry.path);
            },
            .needsGraphics = false
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void AnimationManager::FinalizeResource(EntryType& entry)
    {
        uint32_t entryIndex = _pathToId.at(entry.path);

        auto& gpuData = *(entry.resource->transientGpuData);
        auto& cookedData = *(entry.resource->transientCpuData);
        auto& cpuData = entry.resource->cpuData;

        _cpuExtractor->Extract(cookedData, gpuData, cpuData);

        GpuAnimationAddresses addresses{};
        const auto& hw = entry.resource->hardwareBuffers;

        addresses.vertexSkinData = hw.vertexSkinData->GetDeviceAddress();
        addresses.nodeTransforms = hw.nodeTransforms->GetDeviceAddress();
        addresses.frameGlobalColliders = hw.frameGlobalColliders->GetDeviceAddress();
        addresses.frameMeshColliders = hw.frameMeshColliders->GetDeviceAddress();
        addresses.frameMeshletColliders = hw.frameMeshletColliders->GetDeviceAddress();
        addresses.descriptor = entry.resource->cpuData.descriptor;
		addresses.globalCollider = entry.resource->cpuData.globalCollider;
        addresses.padding = 0;

        WriteAddress(entryIndex, addresses);

        entry.resource->transientGpuData.reset();
        entry.resource->transientCpuData.reset();
    }
}