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
            auto getTexId = [this](const std::string& path) -> uint32_t {
                return path.empty() ? UINT32_MAX : _textureLoadCallback(path);
                };

            Material mat;
            mat.color = info.color;
            mat.emissiveColor = info.emissiveFactor;
            mat.emissiveIntensity = info.emissiveIntensity;
            mat.uvScale = info.uvScale;
            mat.metalness = info.metallicFactor;
            mat.roughness = info.roughnessFactor;
            mat.aoStrength = info.aoStrength;
            mat.useBloom = info.useBloom;
            mat.doubleSided = info.doubleSided;
            mat.isTransparent = info.isTransparent;

            mat.albedoTexture = getTexId(info.albedoPath);
            mat.normalTexture = getTexId(info.normalPath);
            mat.metalnessTexture = getTexId(info.metalnessPath);
            mat.roughnessTexture = getTexId(info.roughnessPath);
            mat.metallicRoughnessTexture = getTexId(info.metallicRoughnessPath);
            mat.emissiveTexture = getTexId(info.emissivePath);
            mat.ambientOcclusionTexture = getTexId(info.ambientOcclusionPath);

            return std::make_shared<Material>(mat);
            });
    }

    void MaterialManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryIndex = _pathToId.at(entry.path);
        size_t offset = entryIndex * sizeof(GpuMaterial);

        auto materialGPU = GpuMaterial(*entry.resource);
        _materialBuffer->Write(&materialGPU, sizeof(GpuMaterial), offset);

        entry.state = ResourceState::Ready;
        _version.fetch_add(1, std::memory_order_release);

        //Info("Material '{}' is ready", entry.path);
    }

    void MaterialManager::LoadDefaultMaterialSync()
    {
        MaterialInfo defaultInfo{};
        uint32_t defaultId = LoadMaterial(MaterialNames::EngineDefault, defaultInfo);

        auto& entry = _entries[defaultId];
        entry.resource = entry.cpuFuture.get();
        entry.state = ResourceState::UploadingGPU;

        StartGpuUpload(entry);
    }

    void MaterialManager::FinalizeResource(EntryType& entry) {
    }
}