#include "MaterialManager.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Material/MaterialNames.h"

namespace Syn {

    MaterialManager::MaterialManager(TextureLoadCallback textureLoadCallback)
        : _textureLoadCallback(std::move(textureLoadCallback))
    {
        _materialBuffer = Vk::BufferFactory::CreatePersistent(
            MAX_MATERIALS * sizeof(GpuMaterial),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
        );

        Material emptyMat;
        GpuMaterial safeDefaultMaterial(emptyMat);
        std::vector<GpuMaterial> safeData(MAX_MATERIALS, safeDefaultMaterial);

        _materialBuffer->Write(safeData.data(), safeData.size() * sizeof(GpuMaterial), 0);

        LoadDefaultMaterialSync();
    }

    uint32_t MaterialManager::LoadMaterial(const std::string& name, const MaterialInfo& info) {
        return InternalLoadSync(name, [this, info]() {
            auto getTexId = [this](const TexturePayload& payload) -> uint32_t {
                if (payload.path.empty() && !payload.IsEmbedded()) 
                    return UINT32_MAX;

                return _textureLoadCallback(payload);
                };

            Material mat;
            mat.color = info.color;
            mat.emissiveColor = info.emissiveFactor;
            mat.emissiveIntensity = info.emissiveIntensity;
            mat.uvScale = info.uvScale;
            mat.metalness = info.metallicFactor;
            mat.roughness = info.roughnessFactor;
            mat.aoStrength = info.aoStrength;
            mat.doubleSided = info.doubleSided;
            mat.isTransparent = info.isTransparent;

            mat.albedoTexture = getTexId(info.albedo);
            mat.normalTexture = getTexId(info.normal);
            mat.metalnessTexture = getTexId(info.metalness);
            mat.roughnessTexture = getTexId(info.roughness);
            mat.metallicRoughnessTexture = getTexId(info.metallicRoughness);
            mat.emissiveTexture = getTexId(info.emissive);
            mat.ambientOcclusionTexture = getTexId(info.ambientOcclusion);

            return std::make_shared<Material>(mat);
            });
    }

    void MaterialManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryIndex = _pathToId.at(entry.path);
        size_t offset = entryIndex * sizeof(GpuMaterial);

        auto materialGPU = GpuMaterial(*entry.resource);
        _materialBuffer->Write(&materialGPU, sizeof(GpuMaterial), offset);

        if (entryIndex >= _renderTypeCache.size()) {
            _renderTypeCache.resize(entryIndex + 1, MaterialRenderType::Opaque1Sided);
        }

        bool isTrans = entry.resource->isTransparent;
        bool isDouble = entry.resource->doubleSided;

        if (isTrans && isDouble)       _renderTypeCache[entryIndex] = MaterialRenderType::Transparent2Sided;
        else if (isTrans && !isDouble) _renderTypeCache[entryIndex] = MaterialRenderType::Transparent1Sided;
        else if (!isTrans && isDouble) _renderTypeCache[entryIndex] = MaterialRenderType::Opaque2Sided;
        else                           _renderTypeCache[entryIndex] = MaterialRenderType::Opaque1Sided;

        entry.state = ResourceState::Ready;
        _version.fetch_add(1, std::memory_order_release);

        //Info("Material '{}' is ready", entry.path);
    }

    void MaterialManager::LoadDefaultMaterialSync()
    {
        MaterialInfo defaultInfo{};
        LoadMaterial(MaterialNames::EngineDefault, defaultInfo);
    }

    void MaterialManager::FinalizeResource(EntryType& entry) {
    }
}