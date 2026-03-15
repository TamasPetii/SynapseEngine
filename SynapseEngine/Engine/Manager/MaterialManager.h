#pragma once
#include "Engine/SynApi.h"
#include "BaseResourceManager.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Material/Material.h"
#include "Engine/Mesh/Data/Common/MaterialInfo.h"

namespace Syn {
    using TextureLoadCallback = std::function<uint32_t(const std::string& fullPath)>;

    class SYN_API MaterialManager : public BaseResourceManager<Material> {
    public:
        static constexpr uint32_t MAX_MATERIALS = 10000;

        MaterialManager(TextureLoadCallback textureLoadCallback);
        ~MaterialManager() = default;

        uint32_t LoadMaterial(const std::string& name, const MaterialInfo& info);
        Vk::Buffer* GetMaterialBuffer() const { return _materialBuffer.get(); }
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;

    private:
        TextureLoadCallback _textureLoadCallback;
        std::unique_ptr<Vk::Buffer> _materialBuffer;
    };
}