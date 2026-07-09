#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/IGpuModelUploader.h"
#include "Engine/Mesh/Converter/ICpuModelExtractor.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Utils/WindowedBuffer.h"

namespace Syn {

    using MaterialLoadCallback = std::function<uint32_t(const std::string& name, const MaterialInfo& info)>;
    using MeshSourceFactory = std::function<std::unique_ptr<IMeshSource>()>;
    using StaticMeshFactory = std::function<std::shared_ptr<StaticMesh>()>;
    using PreviewAllocateCallback = std::function<void(uint32_t resourceId)>;
    using PreviewMarkDirtyCallback = std::function<void(uint32_t resourceId)>;

    class SYN_API ModelManager : public AddressResourceManager<StaticMesh, GpuModelAddresses> {
    public:
        ModelManager(uint32_t framesInFlight, 
            std::shared_ptr<StaticMeshBuilder> builder,
            std::unique_ptr<IGpuModelUploader> uploader,
            MaterialLoadCallback materialLoadCallback = nullptr,
            PreviewAllocateCallback previewAllocateCallback = nullptr,
            PreviewMarkDirtyCallback previewMarkDirtyCallback = nullptr
        );

        ~ModelManager() = default;


        uint32_t LoadModelAsync(const std::string& filePath);
        uint32_t LoadModelFromSourceAsync(const std::string& name, MeshSourceFactory factory);
        uint32_t LoadModelFromStaticMeshAsync(const std::string& name, StaticMeshFactory factory);

        uint32_t LoadModelSync(const std::string& filePath);
        uint32_t LoadModelFromSourceSync(const std::string& name, MeshSourceFactory factory);
        uint32_t LoadModelFromStaticMeshSync(const std::string& name, StaticMeshFactory factory);
    protected:
		void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        MaterialLoadCallback _materialLoadCallback;
        PreviewAllocateCallback _previewAllocateCallback;
        PreviewMarkDirtyCallback _previewMarkDirtyCallback;
        std::shared_ptr<StaticMeshBuilder> _builder;
        std::unique_ptr<IGpuModelUploader> _uploader;
    };
}