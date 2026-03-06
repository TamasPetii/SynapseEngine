#include "ModelManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn
{
    ModelManager::ModelManager(std::unique_ptr<StaticMeshBuilder> builder, std::unique_ptr<IGpuModelUploader> uploader)
        : _builder(std::move(builder)),
        _uploader(std::move(uploader))
    {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        _transferQueue = device->GetTransferQueue();

        if (!_transferQueue) {
            _transferQueue = device->GetGraphicsQueue();
        }

        _transferPool = std::make_unique<Vk::CommandPool>(_transferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }

    uint32_t ModelManager::LoadModelAsync(const std::string& filePath)
    {
        if (_pathToId.contains(filePath)) {
            return _pathToId[filePath];
        }

        uint32_t newId = static_cast<uint32_t>(_models.size());
        _pathToId[filePath] = newId;

        ModelEntry entry;
        entry.path = filePath;
        entry.state = ModelState::LoadingCPU;

        entry.cpuFuture = std::async(std::launch::async, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });

        _models.push_back(std::move(entry));
        return newId;
    }

    void ModelManager::Update()
    {
        for (auto& entry : _models)
        {
            if (entry.state == ModelState::LoadingCPU)
            {
                if (entry.cpuFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    entry.mesh = entry.cpuFuture.get();

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
                    entry.stagingBuffer.reset();
                    entry.transferCmd.reset();
                    entry.uploadFence.reset();
                    entry.state = ModelState::Ready;
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