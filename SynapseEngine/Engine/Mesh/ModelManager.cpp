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

    ModelManager::ModelManager(std::shared_ptr<StaticMeshBuilder> builder, std::unique_ptr<IGpuModelUploader> uploader, MaterialLoadCallback materialLoadCallback)
        : _builder(builder), _uploader(std::move(uploader)), _materialLoadCallback(std::move(materialLoadCallback))
    {
        _modelAddressBuffer = Vk::BufferFactory::CreatePersistent(
            MAX_MODELS * sizeof(GpuModelAddresses),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        );
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
        if (_materialLoadCallback && entry.resource) {
            std::filesystem::path modelDir = std::filesystem::path(entry.path).parent_path();

            std::vector<uint32_t> loadedMaterialIds;
            loadedMaterialIds.reserve(entry.resource->gpuData.materials.size());

            for (auto& matInfo : entry.resource->gpuData.materials) {
                auto resolvePath = [&](std::string& path) {
                    if (!path.empty()) {
                        path = (modelDir / path).lexically_normal().string();
                    }
                    };

                resolvePath(matInfo.albedoPath);
                resolvePath(matInfo.normalPath);
                resolvePath(matInfo.metalnessPath);
                resolvePath(matInfo.roughnessPath);
                resolvePath(matInfo.metallicRoughnessPath);
                resolvePath(matInfo.emissivePath);
                resolvePath(matInfo.ambientOcclusionPath);

                std::string uniqueMatName = (matInfo.name == MaterialNames::EngineDefault)
                    ? matInfo.name
                    : entry.path + "_" + matInfo.name;

                std::mt19937 rng(std::random_device{}());
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);

                uint32_t matId = _materialLoadCallback(uniqueMatName, matInfo);

                loadedMaterialIds.push_back(matId);
            }

            size_t totalDescriptors = entry.resource->gpuData.indexedData.meshDescriptors.size();
            size_t meshCount = totalDescriptors / 4;

            entry.resource->meshMaterialIndices.clear();
            entry.resource->meshMaterialIndices.reserve(meshCount);

            for (size_t i = 0; i < meshCount; ++i) {
                uint32_t localMatIndex = entry.resource->gpuData.indexedData.meshDescriptors[i * 4].materialIndex;

                if (localMatIndex >= loadedMaterialIds.size()) {
                    localMatIndex = 0;
                }

                uint32_t globalMatId = loadedMaterialIds[localMatIndex];
                entry.resource->meshMaterialIndices.push_back(globalMatId);
            }
        }

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

    void ModelManager::FinalizeResource(EntryType& entry)
    {
        auto& gpuData = entry.resource->gpuData;
        size_t totalLodCount = gpuData.indexedData.meshDescriptors.size();

        entry.resource->baseDrawCommands.reserve(totalLodCount);

        for (size_t i = 0; i < totalLodCount; ++i)
        {
            const auto& tradDesc = gpuData.indexedData.meshDescriptors[i];

            bool hasMeshlet = i < gpuData.meshletData.drawDescriptors.size();
            const auto& meshletDesc = hasMeshlet ? gpuData.meshletData.drawDescriptors[i] : GpuMeshletDrawDescriptor{};

            MeshDrawBlueprint blueprint{};

            blueprint.traditionalCmd.vertexCount = tradDesc.indexCount;
            blueprint.traditionalCmd.instanceCount = 0; // GPU/CPU culling fogja növelni!
            blueprint.traditionalCmd.firstVertex = tradDesc.indexOffset;
            blueprint.traditionalCmd.firstInstance = 0;

            /*
            * Mesh shader: blueprint.meshletCmd.groupCountY = meshletDesc.meshletCount;
            * Task shader: blueprint.meshletCmd.groupCountY = (meshletDesc.meshletCount + 31) / 32;
            */

            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(meshletDesc.meshletCount, ComputeGroupSize::Buffer32D);
            blueprint.meshletCmd.groupCountX = 0; // GPU/CPU culling fogja növelni!
            blueprint.meshletCmd.groupCountY = groupCountY;
            blueprint.meshletCmd.groupCountZ = 1;

            /*if (hasMeshlet && tradDesc.indexCount > 5000) { */
            if (true || rand() % 2 == 0) {
                blueprint.isMeshletPipeline = MeshDrawBlueprint::PIPELINE_MESHLET;
            }
            else {
                blueprint.isMeshletPipeline = MeshDrawBlueprint::PIPELINE_TRADITIONAL;
            }

            entry.resource->baseDrawCommands.push_back(blueprint);
        }
        
        uint32_t entryIndex = _pathToId.at(entry.path);

        GpuModelAddresses addresses{};
        const auto& hw = entry.resource->hardwareBuffers;

        auto getAddr = [](const std::unique_ptr<Vk::Buffer>& buf) -> VkDeviceAddress {
            return buf ? buf->GetDeviceAddress() : 0;
            };

        addresses.vertexPositions = getAddr(hw.vertexPositions);
        addresses.vertexAttributes = getAddr(hw.vertexAttributes);
        addresses.indices = getAddr(hw.indices);

        addresses.meshDescriptors = getAddr(hw.meshDescriptors);
        addresses.meshColliders = getAddr(hw.meshColliders);
        addresses.lodDescriptors = getAddr(hw.lodDescriptors);

        addresses.meshletVertexIndices = getAddr(hw.meshletVertexIndices);
        addresses.meshletTriangleIndices = getAddr(hw.meshletTriangleIndices);
        addresses.meshletDescriptors = getAddr(hw.meshletDescriptors);
        addresses.meshletDrawDescriptors = getAddr(hw.meshletDrawDescriptors);
        addresses.meshletColliders = getAddr(hw.meshletColliders);

        addresses.nodeTransforms = getAddr(hw.nodeTransforms);

        addresses.globalCollider = entry.resource->gpuData.globalCollider;
        addresses.vertexCount = entry.resource->gpuData.globalVertexCount;
        addresses.indexCount = entry.resource->gpuData.globalIndexCount;
        addresses.averageLodIndexCount = entry.resource->gpuData.globalAverageLodIndexCount;
        addresses.meshCount = entry.resource->gpuData.globalMeshCount;

        size_t offset = entryIndex * sizeof(GpuModelAddresses);
        _modelAddressBuffer->Write(&addresses, sizeof(GpuModelAddresses), offset);
    }
}