#include "ModelManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Utils/WindowedBuffer.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Material/MaterialNames.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    ModelManager::ModelManager(uint32_t framesInFlight,
        std::shared_ptr<StaticMeshBuilder> builder,
        std::unique_ptr<IGpuModelUploader> uploader,
        MaterialLoadCallback materialLoadCallback,
        PreviewAllocateCallback previewAllocateCallback,
        PreviewMarkDirtyCallback previewMarkDirtyCallback
    )
        : AddressResourceManager<StaticMesh, GpuModelAddresses>(framesInFlight, 1024, 256, 512),
        _builder(builder), 
        _uploader(std::move(uploader)), 
        _materialLoadCallback(std::move(materialLoadCallback)),
        _previewAllocateCallback(std::move(previewAllocateCallback)),
        _previewMarkDirtyCallback(std::move(previewMarkDirtyCallback))
    {
    }

    uint32_t ModelManager::LoadModelAsync(const std::string& filePath) {
        return InternalLoadAsync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t ModelManager::LoadModelFromSourceAsync(const std::string& name, MeshSourceFactory factory) {
        return InternalLoadAsync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<StaticMesh>(nullptr);
            });
    }

    uint32_t ModelManager::LoadModelFromStaticMeshAsync(const std::string& name, StaticMeshFactory factory) {
        return InternalLoadAsync(name, [factory]() {
            return factory();
            });
    }

    uint32_t ModelManager::LoadModelSync(const std::string& filePath) {
        return InternalLoadSync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t ModelManager::LoadModelFromSourceSync(const std::string& name, MeshSourceFactory factory) {
        return InternalLoadSync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<StaticMesh>(nullptr);
            });
    }

    uint32_t ModelManager::LoadModelFromStaticMeshSync(const std::string& name, StaticMeshFactory factory) {
        return InternalLoadSync(name, [factory]() {
            return factory();
            });
    }

    void ModelManager::StartGpuUpload(EntryType& entry) {
        if (_materialLoadCallback && entry.resource && entry.resource->transientGpuData) 
        {
            std::filesystem::path modelDir = std::filesystem::path(entry.path).parent_path();
            auto& transientGpu = *(entry.resource->transientGpuData);

            std::vector<uint32_t> loadedMaterialIds;
            loadedMaterialIds.reserve(transientGpu.materials.size());

            for (auto& matInfo : transientGpu.materials) {
                auto resolvePath = [&](TexturePayload& payload) {
                    if (!payload.path.empty() && !payload.IsEmbedded()) {
                        if (payload.path.empty() || payload.path[0] != '*') {
                            payload.path = (modelDir / payload.path).lexically_normal().string();
                        }
                    }
                    };

                resolvePath(matInfo.albedo);
                resolvePath(matInfo.normal);
                resolvePath(matInfo.metalness);
                resolvePath(matInfo.roughness);
                resolvePath(matInfo.metallicRoughness);
                resolvePath(matInfo.emissive);
                resolvePath(matInfo.ambientOcclusion);

                std::string uniqueMatName = (matInfo.name == MaterialNames::EngineDefault)
                    ? matInfo.name
                    : entry.path + "_" + matInfo.name;

                std::mt19937 rng(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);

                uint32_t matId = _materialLoadCallback(uniqueMatName, matInfo);

                loadedMaterialIds.push_back(matId);
            }

            size_t totalDescriptors = transientGpu.indexedData.meshDescriptors.size();
            size_t meshCount = totalDescriptors / 4;

            entry.resource->cpuData.meshMaterialIndices.clear();
            entry.resource->cpuData.meshMaterialIndices.reserve(meshCount);

            for (size_t i = 0; i < meshCount; ++i) {
                uint32_t localMatIndex = transientGpu.indexedData.meshDescriptors[i * 4].materialIndex;

                if (localMatIndex >= loadedMaterialIds.size()) {
                    localMatIndex = 0;
                }

                uint32_t globalMatId = loadedMaterialIds[localMatIndex];
                entry.resource->cpuData.meshMaterialIndices.push_back(globalMatId);
                transientGpu.meshMaterialIndices.push_back(globalMatId);
            }
        }

        uint32_t entryId = _pathToId.at(entry.path);
        std::shared_ptr<StaticMesh> res = entry.resource;

        Vk::GpuUploadRequest request{
            .uploadCallback = [this, entryId, res](VkCommandBuffer cmd) 
            {
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

                Info("Model loaded, hardware buffers ready and transient RAM freed: {}", safeEntry.path);
            },
            .needsGraphics = false
        };

        SubmitGpuRequest(entry, std::move(request));
    }

    void ModelManager::FinalizeResource(EntryType& entry)
    {
        uint32_t index = _pathToId.at(entry.path);
        if (_previewAllocateCallback) _previewAllocateCallback(index);
        entry.resource->transientGpuData.reset();
        entry.resource->transientCpuData.reset();
    }

    void ModelManager::FlushDirtyResources() 
    {
        ProcessDirtyReadyEntries([this](uint32_t index, const EntryType& entry) {

            GpuModelAddresses addresses{};
            const auto& hw = entry.resource->hardwareBuffers;
            const auto& cpuData = entry.resource->cpuData;

            addresses.vertexPositions = hw.vertexPositions->GetDeviceAddress();
            addresses.vertexAttributes = hw.vertexAttributes->GetDeviceAddress();
            addresses.indices = hw.indices->GetDeviceAddress();
            addresses.meshMaterialIndices = hw.meshMaterialIndices->GetDeviceAddress();
            addresses.meshDescriptors = hw.meshDescriptors->GetDeviceAddress();
            addresses.meshColliders = hw.meshColliders->GetDeviceAddress();
            addresses.lodDescriptors = hw.lodDescriptors->GetDeviceAddress();
            addresses.meshletVertexIndices = hw.meshletVertexIndices->GetDeviceAddress();
            addresses.meshletTriangleIndices = hw.meshletTriangleIndices->GetDeviceAddress();
            addresses.meshletDescriptors = hw.meshletDescriptors->GetDeviceAddress();
            addresses.meshletDrawDescriptors = hw.meshletDrawDescriptors->GetDeviceAddress();
            addresses.meshletColliders = hw.meshletColliders->GetDeviceAddress();
            addresses.nodeTransforms = hw.nodeTransforms->GetDeviceAddress();

            addresses.globalCollider = cpuData.globalCollider;
            addresses.vertexCount = cpuData.globalVertexCount;
            addresses.indexCount = cpuData.globalIndexCount;
            addresses.meshCount = cpuData.globalMeshCount;
            addresses.averageLodIndexCount = cpuData.globalAverageLodIndexCount;
            addresses.isReady = 1;

            WriteAddress(index, addresses);

            if (_previewMarkDirtyCallback) _previewMarkDirtyCallback(index);
            });
    }

    std::vector<uint32_t> ModelManager::GetModelsUsingMaterials(uint32_t materialId) const {
        std::lock_guard lock(_mutex);
        std::vector<uint32_t> result;

        for (uint32_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i].state == ResourceState::Ready && _entries[i].resource) {
                const auto& model = *_entries[i].resource;

                for (auto meshMaterialIndex : model.cpuData.meshMaterialIndices)
                {
                    if (meshMaterialIndex == materialId)
                    {
                        result.push_back(i);
                        break;
                    }
                }
            }
        }

        return result;
    }

    void ModelManager::NotifyMaterialReady(uint32_t materialId) {
        std::lock_guard lock(_pendingMaterialMutex);
        _pendingMaterials.insert(materialId);
    }

    void ModelManager::ProcessPendingNotifications() {
        std::unordered_set<uint32_t> materialsToProcess;

        {
            std::lock_guard lock(_pendingMaterialMutex);
            materialsToProcess.swap(_pendingMaterials);
        }

        if (materialsToProcess.empty()) return;

        for (uint32_t materialId : materialsToProcess) {
            for (uint32_t modelId : GetModelsUsingMaterials(materialId)) {
                if (_previewMarkDirtyCallback) 
                        _previewMarkDirtyCallback(modelId);
            }
        }
    }
}