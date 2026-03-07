#include "ModelManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/SynMacro.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    ModelManager::ModelManager(
        std::shared_ptr<StaticMeshBuilder> builder,
        std::unique_ptr<IGpuModelUploader> uploader,
        TextureLoadCallback textureLoadCallback)
        : 
        _builder(builder),
        _uploader(std::move(uploader)),
        _textureLoadCallback(std::move(textureLoadCallback))
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        _transferQueue = device->GetTransferQueue();

        if (!_transferQueue) {
            _transferQueue = device->GetGraphicsQueue();
        }

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }

    uint32_t ModelManager::InternalLoadAsync(const std::string& key, ModelLoadTask task)
    {
        if (_pathToId.contains(key)) {
            return _pathToId[key];
        }

        uint32_t newId = static_cast<uint32_t>(_models.size());
        _pathToId[key] = newId;

        ModelEntry entry;
        entry.path = key;
        entry.state = ModelState::LoadingCPU;

        auto executor = ServiceLocator::GetTaskExecutor();
        entry.cpuFuture = executor->async(std::move(task));

        _models.push_back(std::move(entry));
        return newId;
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

    void ModelManager::Update()
    {
        for (auto& entry : _models)
        {
            if (entry.state == ModelState::LoadingCPU)
            {
                if (entry.cpuFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
					Info("Model loaded from file: {}", entry.path);

                    entry.mesh = entry.cpuFuture.get();

                    if (_textureLoadCallback && entry.mesh)
                    {
                        std::filesystem::path modelDir = std::filesystem::path(entry.path).parent_path();

                        for (const auto& mat : entry.mesh->cpuData.materials)
                        {
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

                    entry.transferCmd = _transferPool->AllocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
                    entry.uploadFence = std::make_unique<Vk::Fence>(false);

                    entry.transferCmd->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                    auto uploadResult = _uploader->Upload(entry.mesh->gpuData, entry.transferCmd->Handle());
                    entry.transferCmd->End();

                    entry.mesh->hardwareBuffers = std::move(uploadResult.hardwareBuffers);
                    entry.stagingBuffer = std::move(uploadResult.stagingBuffer);

                    VkCommandBufferSubmitInfo cmdSubmitInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
                    cmdSubmitInfo.commandBuffer = entry.transferCmd->Handle();

                    VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
                    submitInfo.commandBufferInfoCount = 1;
                    submitInfo.pCommandBufferInfos = &cmdSubmitInfo;

                    _transferQueue->Submit(&submitInfo, entry.uploadFence->Handle());

                    entry.state = ModelState::UploadingGPU;
                }
            }
            else if (entry.state == ModelState::UploadingGPU)
            {
                if (entry.uploadFence->IsSignaled())
                {
                    Info("Model uploaded to GPU: {}", entry.path);

                    uint32_t meshLodCount = entry.mesh->gpuData.indexedData.meshDescriptors.size();

                    {
                        std::vector<VkDrawIndirectCommand> indirectCommands;
                        indirectCommands.reserve(meshLodCount);

                        int index = 0;
                        for (auto& desc : entry.mesh->gpuData.indexedData.meshDescriptors)
                        {
                            VkDrawIndirectCommand cmd{};
                            cmd.vertexCount = desc.indexCount;
                            cmd.instanceCount = index % 4 == 0 ? 1 : 0;
                            cmd.firstVertex = desc.indexOffset;
                            cmd.firstInstance = 0; //??

                            indirectCommands.push_back(cmd);
                            index++;
                        }

                        entry.mesh->hardwareBuffers.indirectBuffer = Vk::BufferFactory::CreatePersistent(
                            indirectCommands.size() * sizeof(VkDrawIndirectCommand),
                            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                        );

                        entry.mesh->hardwareBuffers.indirectBuffer->Write(
                            indirectCommands.data(),
                            indirectCommands.size() * sizeof(VkDrawIndirectCommand)
                        );
                    }

                    {
                        std::vector<VkDrawMeshTasksIndirectCommandEXT> meshIndirectCommands;
                        meshIndirectCommands.reserve(meshLodCount);

                        int index = 0;
                        for (auto& desc : entry.mesh->gpuData.meshletData.drawDescriptors)
                        {
                            VkDrawMeshTasksIndirectCommandEXT cmd{};
                            cmd.groupCountX = index % 4 == 0 ? desc.meshletCount : 0;
                            cmd.groupCountY = 1;
                            cmd.groupCountZ = 1;

                            meshIndirectCommands.push_back(cmd);
                            index++;
                        }

                        entry.mesh->hardwareBuffers.indirectMeshletBuffer = Vk::BufferFactory::CreatePersistent(
                            meshIndirectCommands.size() * sizeof(VkDrawMeshTasksIndirectCommandEXT),
                            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                        );

                        entry.mesh->hardwareBuffers.indirectMeshletBuffer->Write(
                            meshIndirectCommands.data(),
                            meshIndirectCommands.size() * sizeof(VkDrawMeshTasksIndirectCommandEXT)
                        );
                    }
                    
                    entry.stagingBuffer.reset();
                    entry.transferCmd.reset();
                    entry.uploadFence.reset();
                    entry.state = ModelState::Ready;

                    //Todo: Buffer Address Buffer
                }
            }
        }
    }

    std::shared_ptr<StaticMesh> ModelManager::GetModel(uint32_t id) const
    {
        if (id >= _models.size()) 
            return nullptr;

        if (_models[id].state == ModelState::Ready) 
            return _models[id].mesh;

        return nullptr;
    }

    std::shared_ptr<StaticMesh> ModelManager::GetModel(const std::string& filePath) const
    {
		if (_pathToId.find(filePath) == _pathToId.end())
            return nullptr;

		return GetModel(_pathToId.at(filePath));
    }
}