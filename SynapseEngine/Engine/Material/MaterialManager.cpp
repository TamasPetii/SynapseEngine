#include "MaterialManager.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Material/MaterialNames.h"

namespace Syn {

    MaterialManager::MaterialManager(uint32_t framesInFlight, 
        TextureLoadCallback textureLoadCallback, 
        PreviewAllocateCallback previewAllocateCallback,
        PreviewMarkDirtyCallback previewMarkDirtyCallback
    )
        : AddressResourceManager<Material, GpuMaterial>(framesInFlight, 1024, 1024, 2048)
        , _textureLoadCallback(std::move(textureLoadCallback))
        , _previewAllocateCallback(std::move(previewAllocateCallback))
        , _previewMarkDirtyCallback(std::move(previewMarkDirtyCallback))
    {
        Material emptyMat;
        WriteAddress(0, GpuMaterial(emptyMat));
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

    uint32_t MaterialManager::LoadMaterialDirect(const std::string& name, const Material& material) {
        return InternalLoadSync(name, [this, material]() {
            return std::make_shared<Material>(material);
            });
    }

    void MaterialManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryIndex = _pathToId.at(entry.path);

        FinalizeResource(entry);

        entry.state = ResourceState::Ready;
        MarkDirty(entryIndex);

        Info("Material '{}' is ready", entry.path);
    }

    void MaterialManager::LoadDefaultMaterialSync()
    {
        MaterialInfo defaultInfo{};
        LoadMaterial(MaterialNames::EngineDefault, defaultInfo);
    }

    void MaterialManager::FinalizeResource(EntryType& entry) {
        uint32_t index = _pathToId.at(entry.path);
        if (_previewAllocateCallback) _previewAllocateCallback(index);
        if (_previewMarkDirtyCallback) _previewMarkDirtyCallback(index);
    }

    void MaterialManager::FlushDirtyResources() 
    {
        std::vector<uint32_t> imagesToProcess;

        {
            std::lock_guard lock(_pendingImageMutex);
            imagesToProcess = _pendingImages;
            _pendingImages.clear();
        }

        for (uint32_t imgId : imagesToProcess) {
            auto affectedMaterials = GetMaterialsUsingTexture(imgId);

            for (uint32_t matId : affectedMaterials) {
                if (_previewMarkDirtyCallback) {
                    _previewMarkDirtyCallback(matId);
                }
            }
        }

        ProcessDirtyReadyEntries(
            [this](uint32_t index, const EntryType& entry) {
                WriteAddress(index, GpuMaterial(*entry.resource));

                if (_previewMarkDirtyCallback) {
                    _previewMarkDirtyCallback(index);
                }
            }
        );
    }

    std::vector<uint32_t> MaterialManager::GetMaterialsUsingTexture(uint32_t textureId) const {
        std::lock_guard lock(_mutex);
        std::vector<uint32_t> result;

        for (uint32_t i = 0; i < _entries.size(); ++i) {
            if (_entries[i].state == ResourceState::Ready && _entries[i].resource) {
                const auto& mat = *_entries[i].resource;

                if (mat.albedoTexture == textureId ||
                    mat.normalTexture == textureId ||
                    mat.metalnessTexture == textureId ||
                    mat.roughnessTexture == textureId ||
                    mat.metallicRoughnessTexture == textureId ||
                    mat.emissiveTexture == textureId ||
                    mat.ambientOcclusionTexture == textureId)
                {
                    result.push_back(i);
                }
            }
        }
        return result;
    }

    void MaterialManager::NotifyImageReady(uint32_t imageId) {
        std::lock_guard lock(_pendingImageMutex);
        _pendingImages.push_back(imageId);
    }
}