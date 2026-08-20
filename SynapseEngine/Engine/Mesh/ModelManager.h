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
    using MeshSourceFactory = std::function<std::unique_ptr<IMeshSource>()>;
    using StaticMeshFactory = std::function<std::shared_ptr<StaticMesh>()>;

    struct ModelManagerCallbacks {
        std::function<uint32_t(const std::string&, const MaterialInfo&)> materialLoad;
        std::function<void(uint32_t)> previewAllocate;
        std::function<void(uint32_t)> previewMarkDirty;
    };

    class SYN_API ModelManager : public AddressResourceManager<StaticMesh, GpuModelAddresses> {
    public:
        ModelManager(uint32_t framesInFlight,
            std::shared_ptr<StaticMeshBuilder> builder,
            std::unique_ptr<IGpuModelUploader> uploader,
            ModelManagerCallbacks callbacks
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
        std::shared_ptr<StaticMeshBuilder> _builder;
        std::unique_ptr<IGpuModelUploader> _uploader;
        ModelManagerCallbacks _callbacks;

        std::mutex _pendingMaterialMutex;
        std::unordered_set<uint32_t> _pendingMaterials;
    };
}