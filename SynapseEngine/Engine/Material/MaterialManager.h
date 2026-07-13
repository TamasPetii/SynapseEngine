#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Material/Material.h"
#include "Engine/Mesh/Data/Common/MaterialInfo.h"
#include "MaterialRenderType.h"

#include <unordered_set>

namespace Syn {
    using TextureLoadCallback = std::function<uint32_t(const TexturePayload& payload)>;
    using PreviewAllocateCallback = std::function<void(uint32_t resourceId)>;
    using PreviewMarkDirtyCallback = std::function<void(uint32_t resourceId)>;
    using MaterialReadyOrChangedCallback = std::function<void(uint32_t materialId)>;

    class SYN_API MaterialManager : public AddressResourceManager<Material, GpuMaterial> {
    public:
        MaterialManager(uint32_t framesInFlight,
            TextureLoadCallback textureLoadCallback,
            PreviewAllocateCallback previewAllocateCallback = nullptr,
            PreviewMarkDirtyCallback previewMarkDirtyCallback = nullptr,
            MaterialReadyOrChangedCallback = nullptr
        );
        ~MaterialManager() = default;

        uint32_t LoadMaterial(const std::string& name, const MaterialInfo& info);
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
    private:
        TextureLoadCallback _textureLoadCallback;
        PreviewAllocateCallback _previewAllocateCallback;
        PreviewMarkDirtyCallback _previewMarkDirtyCallback;
        MaterialReadyOrChangedCallback _materialReadyOrChangedCallback;

        std::mutex _pendingImageMutex;
        std::unordered_set<uint32_t> _pendingImages;
    };
}