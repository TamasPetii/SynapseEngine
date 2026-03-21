#include "AnimationManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    AnimationManager::AnimationManager(std::shared_ptr<AnimationBuilder> builder, std::unique_ptr<IGpuAnimationUploader> uploader)
        : _builder(builder), _uploader(std::move(uploader))
    {
        _animationAddressBuffer = Vk::BufferFactory::CreatePersistent(
            MAX_ANIMATIONS * sizeof(GpuAnimationAddresses),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        );
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

    void AnimationManager::StartGpuUpload(EntryType& entry) {
        Vk::GpuUploadRequest request{
            .uploadCallback = [this, &entry](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(entry.resource->gpuData, cmd);
                entry.resource->hardwareBuffers = std::move(uploadResult.hardwareBuffers);
                entry.stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, &entry]() {
                FinalizeResource(entry);
                entry.stagingBuffer.reset();
                entry.state = ResourceState::Ready;
                _version.fetch_add(1, std::memory_order_release);
            },
            .needsGraphics = false
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void AnimationManager::FinalizeResource(EntryType& entry)
    {
        uint32_t entryIndex = _pathToId.at(entry.path);

        GpuAnimationAddresses addresses{};
        const auto& hw = entry.resource->hardwareBuffers;

        auto getAddr = [](const std::unique_ptr<Vk::Buffer>& buf) -> VkDeviceAddress {
            return buf ? buf->GetDeviceAddress() : 0;
            };

        addresses.vertexSkinData = getAddr(hw.vertexSkinData);
        addresses.nodeTransforms = getAddr(hw.nodeTransforms);
        addresses.frameGlobalColliders = getAddr(hw.frameGlobalColliders);
        addresses.frameMeshColliders = getAddr(hw.frameMeshColliders);
        addresses.frameMeshletColliders = getAddr(hw.frameMeshletColliders);
        addresses.descriptor = entry.resource->gpuData.descriptor;
        addresses.padding = 0;

        size_t offset = entryIndex * sizeof(GpuAnimationAddresses);
        _animationAddressBuffer->Write(&addresses, sizeof(GpuAnimationAddresses), offset);
    }
}