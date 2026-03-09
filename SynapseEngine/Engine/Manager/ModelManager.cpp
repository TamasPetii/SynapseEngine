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

    void ModelManager::FinalizeResource(EntryType& entry) {
        uint32_t meshLodCount = entry.resource->gpuData.indexedData.meshDescriptors.size();

        {
            std::vector<VkDrawIndirectCommand> indirectCommands;
            indirectCommands.reserve(meshLodCount);

            int index = 0;
            for (auto& desc : entry.resource->gpuData.indexedData.meshDescriptors) {
                VkDrawIndirectCommand cmd{};
                cmd.vertexCount = desc.indexCount;
                cmd.instanceCount = index % 4 == 0 ? 1 : 0;
                cmd.firstVertex = desc.indexOffset;
                cmd.firstInstance = 0;

                indirectCommands.push_back(cmd);
                index++;
            }

            entry.resource->hardwareBuffers.indirectBuffer = Vk::BufferFactory::CreatePersistent(
                indirectCommands.size() * sizeof(VkDrawIndirectCommand),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            entry.resource->hardwareBuffers.indirectBuffer->Write(
                indirectCommands.data(),
                indirectCommands.size() * sizeof(VkDrawIndirectCommand)
            );
        }

        {
            std::vector<VkDrawMeshTasksIndirectCommandEXT> meshIndirectCommands;
            meshIndirectCommands.reserve(meshLodCount);

            int index = 0;
            for (auto& desc : entry.resource->gpuData.meshletData.drawDescriptors) {
                VkDrawMeshTasksIndirectCommandEXT cmd{};
                cmd.groupCountX = index % 4 == 0 ? desc.meshletCount : 0;
                cmd.groupCountY = 1;
                cmd.groupCountZ = 1;

                meshIndirectCommands.push_back(cmd);
                index++;
            }

            entry.resource->hardwareBuffers.indirectMeshletBuffer = Vk::BufferFactory::CreatePersistent(
                meshIndirectCommands.size() * sizeof(VkDrawMeshTasksIndirectCommandEXT),
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
            );

            entry.resource->hardwareBuffers.indirectMeshletBuffer->Write(
                meshIndirectCommands.data(),
                meshIndirectCommands.size() * sizeof(VkDrawMeshTasksIndirectCommandEXT)
            );
        }
    }
}