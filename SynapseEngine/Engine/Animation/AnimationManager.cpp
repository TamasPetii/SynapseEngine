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
        : AddressResourceManager<Animation, GpuAnimationAddresses>(framesInFlight, 1024, 256, 512),
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

            return _builder->BuildFromFile(filePath, baseModel->cpuData);
            });
    }

    uint32_t AnimationManager::LoadAnimationSync(const std::string& filePath, uint32_t baseModelId) {
        std::string uniqueName = filePath + "_" + std::to_string(baseModelId);

        return InternalLoadSync(uniqueName, [this, filePath, baseModelId]() {
            auto modelManager = ServiceLocator::GetModelManager();
            auto baseModel = modelManager->GetResource(baseModelId);

            if (!baseModel) 
                return std::shared_ptr<Animation>(nullptr);

            return _builder->BuildFromFile(filePath, baseModel->cpuData);
            });
    }

    void AnimationManager::StartGpuUpload(EntryType& entry) 
    {
        uint32_t entryId = _pathToId.at(entry.path);
        std::shared_ptr<Animation> res = entry.resource;

        Vk::GpuUploadRequest request{
            .uploadCallback = [this, entryId, res](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(*(res->transientGpuData), cmd);

                std::lock_guard lock(_mutex);
                auto& safeEntry = _entries[entryId];
                safeEntry.resource->hardwareBuffers = std::move(uploadResult.hardwareBuffers);
                safeEntry.stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, entryId]() {
                std::lock_guard lock(_mutex);
                auto& safeEntry = _entries[entryId];

                FinalizeResource(safeEntry);
                safeEntry.stagingBuffer.reset();

                SetResourceState(entryId, ResourceState::Ready);
                MarkDirty(entryId);

                Info("Animation loaded, extracted, and RAM freed: {}", safeEntry.path);
            },
            .needsGraphics = false
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void AnimationManager::FinalizeResource(EntryType& entry)
    {
        auto& gpuData = *(entry.resource->transientGpuData);
        auto& cpuData = entry.resource->cpuData;

        _cpuExtractor->Extract(gpuData, cpuData);

        entry.resource->transientGpuData.reset();
        entry.resource->transientCpuData.reset();
    }

    void AnimationManager::FlushDirtyResources()
    {
        ProcessDirtyReadyEntries([this](uint32_t index, const EntryType& entry) {
            GpuAnimationAddresses addresses{};
            const auto& hw = entry.resource->hardwareBuffers;

            addresses.vertexSkinData = hw.vertexSkinData->GetDeviceAddress();
            addresses.nodeTransforms = hw.nodeTransforms->GetDeviceAddress();
            addresses.frameGlobalColliders = hw.frameGlobalColliders->GetDeviceAddress();
            addresses.frameMeshColliders = hw.frameMeshColliders->GetDeviceAddress();
            addresses.frameMeshletColliders = hw.frameMeshletColliders->GetDeviceAddress();

            addresses.descriptor = entry.resource->cpuData.descriptor;
            addresses.globalCollider = entry.resource->cpuData.globalCollider;
            addresses.isReady = 1;

            WriteAddress(index, addresses);
            });
    }
}