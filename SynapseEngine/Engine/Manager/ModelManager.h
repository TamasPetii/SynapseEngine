#pragma once
#include "Engine/SynApi.h"
#include "BaseResourceManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/IGpuModelUploader.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"

namespace Syn {

    using TextureLoadCallback = std::function<void(const std::string& fullPath)>;
    using MeshSourceFactory = std::function<std::unique_ptr<IMeshSource>()>;
    using StaticMeshFactory = std::function<std::shared_ptr<StaticMesh>()>;

    class SYN_API ModelManager : public BaseResourceManager<StaticMesh> {
    public:
        ModelManager(std::shared_ptr<StaticMeshBuilder> builder, std::unique_ptr<IGpuModelUploader> uploader, TextureLoadCallback textureLoadCallback = nullptr);
        ~ModelManager() = default;

        uint32_t LoadModelAsync(const std::string& filePath);
        uint32_t LoadModelFromSourceAsync(const std::string& name, MeshSourceFactory factory);
        uint32_t LoadModelFromStaticMeshAsync(const std::string& name, StaticMeshFactory factory);
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        TextureLoadCallback _textureLoadCallback;
        std::shared_ptr<StaticMeshBuilder> _builder;
        std::unique_ptr<IGpuModelUploader> _uploader;
    };
}