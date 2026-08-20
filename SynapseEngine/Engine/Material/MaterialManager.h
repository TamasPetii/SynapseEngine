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
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Material/Material.h"
#include "Engine/Mesh/Data/Common/MaterialInfo.h"
#include "MaterialRenderType.h"
#include <unordered_set>

namespace Syn {
    struct MaterialManagerCallbacks {
        std::function<uint32_t(const TexturePayload&)> textureLoad;
        std::function<void(uint32_t)> previewAllocate;
        std::function<void(uint32_t)> previewMarkDirty;
        std::function<void(uint32_t)> materialReadyOrChanged;
    };

    class SYN_API MaterialManager : public AddressResourceManager<Material, GpuMaterial> {
    public:
        MaterialManager(uint32_t framesInFlight,
            MaterialManagerCallbacks callbacks
        );
        ~MaterialManager() = default;

        uint32_t LoadMaterialAsync(const std::string& name, const MaterialInfo& info);
        uint32_t LoadMaterialSync(const std::string& name, const MaterialInfo& info);

        uint32_t LoadMaterialDirect(const std::string& name, const Material& material);
        std::vector<uint32_t> GetMaterialsUsingTexture(uint32_t textureId) const;
        void NotifyImageReady(uint32_t imageId);
        void ProcessPendingNotifications() override;
    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        void LoadDefaultMaterialSync();
        std::shared_ptr<Material> CreateMaterialFromInfo(const MaterialInfo& info);
    private:
        MaterialManagerCallbacks _callbacks;

        std::mutex _pendingImageMutex;
        std::unordered_set<uint32_t> _pendingImages;
    };
}