#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/IGpuModelUploader.h"
#include <vector>
#include <memory>
#include <string>
#include <future>
#include <unordered_map>

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Vk/Command/CommandBuffer.h"
#include "Engine/Vk/Synchronization/Fence.h"

namespace Syn
{
    enum class SYN_API ModelState {
        LoadingCPU,
        UploadingGPU,
        Ready
    };

    struct SYN_API ModelEntry {
        ModelState state;

        std::string path;
        std::shared_ptr<StaticMesh> mesh;

        std::unique_ptr<Vk::Fence> uploadFence;
        std::unique_ptr<Vk::Buffer> stagingBuffer;
        std::unique_ptr<Vk::CommandBuffer> transferCmd;

        std::future<std::shared_ptr<StaticMesh>> cpuFuture;
    };

    class SYN_API ModelManager
    {
    public:
        ModelManager(std::shared_ptr<StaticMeshBuilder> builder, std::unique_ptr<IGpuModelUploader> uploader);
        ~ModelManager() = default;

        uint32_t LoadModelAsync(const std::string& filePath);
        void Update();
        std::shared_ptr<StaticMesh> GetModel(uint32_t id) const;
		std::shared_ptr<StaticMesh> GetModel(const std::string& filePath) const;
    private:
        std::shared_ptr<StaticMeshBuilder> _builder;
        std::unique_ptr<IGpuModelUploader> _uploader;

        Vk::ThreadSafeQueue* _transferQueue = nullptr;
        std::unique_ptr<Vk::CommandPool> _transferPool;

        std::vector<ModelEntry> _models;
        std::unordered_map<std::string, uint32_t> _pathToId;
    };
}