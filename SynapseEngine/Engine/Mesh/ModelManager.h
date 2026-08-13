// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/IGpuModelUploader.h"
#include "Engine/Mesh/Converter/ICpuModelExtractor.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Utils/WindowedBuffer.h"

#include <unordered_set>

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

        std::vector<uint32_t> GetModelsUsingMaterials(uint32_t materialId) const;
        void NotifyMaterialReady(uint32_t materialId);
        void ProcessPendingNotifications() override;
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

        std::mutex _pendingMaterialMutex;
        std::unordered_set<uint32_t> _pendingMaterials;
    };
}