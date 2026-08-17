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

#include "MaterialManager.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Material/MaterialNames.h"

namespace Syn {

    MaterialManager::MaterialManager(uint32_t framesInFlight,
        MaterialManagerCallbacks callbacks
    )
        : AddressResourceManager<Material, GpuMaterial>(framesInFlight, 1024, 1024, 2048)
        , _callbacks(std::move(callbacks))
    {
        Material emptyMat;
        WriteAddress(0, GpuMaterial(emptyMat));
        LoadDefaultMaterialSync();
    }

    std::shared_ptr<Material> MaterialManager::CreateMaterialFromInfo(const MaterialInfo& info)
    {
        auto getTexId = [this](const TexturePayload& payload) -> uint32_t {
            if (payload.path.empty() && !payload.IsEmbedded())
                return UINT32_MAX;

            return _callbacks.textureLoad(payload);
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
        mat.isAlphaTested = info.isAlphaTested;
        mat.clearcoatFactor = info.clearcoatFactor;
        mat.clearcoatRoughness = info.clearcoatRoughnessFactor;
        mat.ior = info.ior;
        mat.specularFactor = info.specularFactor;
        mat.specularColor = info.specularColorFactor;

        mat.albedoTexture = getTexId(info.albedo);
        mat.normalTexture = getTexId(info.normal);
        mat.metalnessTexture = getTexId(info.metalness);
        mat.roughnessTexture = getTexId(info.roughness);
        mat.metallicRoughnessTexture = getTexId(info.metallicRoughness);
        mat.emissiveTexture = getTexId(info.emissive);
        mat.ambientOcclusionTexture = getTexId(info.ambientOcclusion);
        mat.opacityTexture = getTexId(info.opacity);
        mat.clearcoatTexture = getTexId(info.clearcoat);
        mat.clearcoatRoughnessTexture = getTexId(info.clearcoatRoughness);
        mat.clearcoatNormalTexture = getTexId(info.clearcoatNormal);
        mat.specularTexture = getTexId(info.specular);
        mat.specularColorTexture = getTexId(info.specularColor);

        return std::make_shared<Material>(mat);
    }

    uint32_t MaterialManager::LoadMaterialSync(const std::string& name, const MaterialInfo& info) {
        return InternalLoadSync(name, [this, info]() {
            return CreateMaterialFromInfo(info);
            });
    }

    uint32_t MaterialManager::LoadMaterialAsync(const std::string& name, const MaterialInfo& info) {
        return InternalLoadAsync(name, [this, info]() {
            return CreateMaterialFromInfo(info);
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
        LoadMaterialSync(MaterialNames::EngineDefault, defaultInfo);
    }

    void MaterialManager::FinalizeResource(EntryType& entry) {
        uint32_t index = _pathToId.at(entry.path);
        if (_callbacks.previewAllocate) _callbacks.previewAllocate(index);
        if (_callbacks.previewMarkDirty) _callbacks.previewMarkDirty(index);
        if (_callbacks.materialReadyOrChanged) _callbacks.materialReadyOrChanged(index);
    }

    void MaterialManager::FlushDirtyResources()
    {
        ProcessDirtyReadyEntries(
            [this](uint32_t index, const EntryType& entry) {
                WriteAddress(index, GpuMaterial(*entry.resource));

                if (_callbacks.previewMarkDirty)
                    _callbacks.previewMarkDirty(index);

                if (_callbacks.materialReadyOrChanged)
                    _callbacks.materialReadyOrChanged(index);
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
        _pendingImages.insert(imageId);
    }

    void MaterialManager::ProcessPendingNotifications() {
        std::unordered_set<uint32_t> imagesToProcess;

        {
            std::lock_guard lock(_pendingImageMutex);
            imagesToProcess.swap(_pendingImages);
        }

        if (imagesToProcess.empty()) return;

        for (uint32_t imgId : imagesToProcess) {
            for (uint32_t matId : GetMaterialsUsingTexture(imgId))
            {
                if (_callbacks.previewMarkDirty)
                    _callbacks.previewMarkDirty(matId);

                if (_callbacks.materialReadyOrChanged)
                    _callbacks.materialReadyOrChanged(matId);
            }
        }
    }
}