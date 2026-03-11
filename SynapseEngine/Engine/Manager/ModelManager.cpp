#include "ModelManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    ModelManager::ModelManager(std::shared_ptr<StaticMeshBuilder> builder, std::unique_ptr<IGpuModelUploader> uploader, TextureLoadCallback textureLoadCallback)
        : _builder(builder), _uploader(std::move(uploader)), _textureLoadCallback(std::move(textureLoadCallback))
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

    void ModelManager::StartGpuUpload(EntryType& entry) {
        if (_textureLoadCallback && entry.resource) {
            std::filesystem::path modelDir = std::filesystem::path(entry.path).parent_path();

            for (const auto& mat : entry.resource->cpuData.materials) {
                auto submitTexture = [&](const std::string& texPath) {
                    if (!texPath.empty()) {
                        std::string fullPath = (modelDir / texPath).lexically_normal().string();
                        _textureLoadCallback(fullPath);
                    }
                    };

                submitTexture(mat.albedoPath);
                submitTexture(mat.normalPath);
                submitTexture(mat.metallicRoughnessPath);
                submitTexture(mat.emissivePath);
                submitTexture(mat.ambientOcclusionPath);
            }
        }

        ServiceLocator::GetGpuUploader()->Enqueue({
            .uploadCallback = [this, &entry](VkCommandBuffer cmd) {
                auto uploadResult = _uploader->Upload(entry.resource->gpuData, cmd);
                entry.resource->hardwareBuffers = std::move(uploadResult.hardwareBuffers);
                entry.stagingBuffer = std::move(uploadResult.stagingBuffer);
            },
            .onFinished = [this, &entry]() {
                FinalizeResource(entry);
                entry.stagingBuffer.reset();
                entry.state = ResourceState::Ready;
                Info("Model '{}' is ready", entry.path);
            },
            .needsGraphics = false
        });
    }

    void ModelManager::FinalizeResource(EntryType& entry)
    {
        auto& gpuData = entry.resource->gpuData;
        size_t totalLodCount = gpuData.indexedData.meshDescriptors.size();

        entry.resource->hardwareBuffers.baseDrawCommands.reserve(totalLodCount);

        for (size_t i = 0; i < totalLodCount; ++i)
        {
            const auto& tradDesc = gpuData.indexedData.meshDescriptors[i];

            bool hasMeshlet = i < gpuData.meshletData.drawDescriptors.size();
            const auto& meshletDesc = hasMeshlet ? gpuData.meshletData.drawDescriptors[i] : GpuMeshletDrawDescriptor{};

            MeshDrawBlueprint blueprint{};

            blueprint.traditionalCmd.vertexCount = tradDesc.indexCount;
            blueprint.traditionalCmd.instanceCount = 0;
            blueprint.traditionalCmd.firstVertex = tradDesc.indexOffset;
            blueprint.traditionalCmd.firstInstance = 0;

            blueprint.meshletCmd.groupCountX = meshletDesc.meshletCount; // GPU fogja növelni?? = 0!
            blueprint.meshletCmd.groupCountY = 1;
            blueprint.meshletCmd.groupCountZ = 1;

            if (false && hasMeshlet && tradDesc.indexCount > 5000) {
                blueprint.isMeshletPipeline = MeshDrawBlueprint::PIPELINE_MESHLET;
            }
            else {
                blueprint.isMeshletPipeline = MeshDrawBlueprint::PIPELINE_TRADITIONAL;
            }

            entry.resource->hardwareBuffers.baseDrawCommands.push_back(blueprint);
        }
    }
}