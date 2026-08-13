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

#include "ManifestSceneWriter.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Serialization/Schema/Scene/SceneSnapshotTypes.h"
#include "Engine/Video/VideoManager.h"

namespace Syn
{
    bool ManifestSceneWriter::SaveScene(Scene& scene, const std::filesystem::path& path)
    {
        std::vector<std::string> modelManifest;
        std::vector<uint32_t> localToGlobalModels;

        std::vector<AnimationManifestEntry> animManifest;
        std::vector<uint32_t> localToGlobalAnims;

        std::vector<MaterialManifestEntry> matManifest;
        std::vector<uint32_t> localToGlobalMats;

		std::vector<TextureManifestEntry> textureManifest;
		std::vector<uint32_t> outLocalToGlobalTex;

        std::vector<VideoManifestEntry> videoManifest;
        std::vector<uint32_t> outLocalToGlobalVideo;

		ProcessMaterials(scene, matManifest, localToGlobalMats);
		ProcessTextures(matManifest, textureManifest, outLocalToGlobalTex);
        ProcessVideos(matManifest, videoManifest, outLocalToGlobalVideo);
        ProcessModels(scene, modelManifest, localToGlobalModels);
        ProcessAnimations(scene, animManifest, localToGlobalAnims);

        FullSceneSnapshot snapshot{ scene };
        snapshot.modelManifest = std::move(modelManifest);
        snapshot.animationManifest = std::move(animManifest);
        snapshot.materialManifest = std::move(matManifest);
		snapshot.textureManifest = std::move(textureManifest);
        snapshot.videoManifest = std::move(videoManifest);

        auto serializer = ServiceLocator::Get<Serializer>();
        bool success = false;
        if (serializer)
            success = serializer->SaveToFile(path, snapshot);

        RestoreOriginalIndices(scene, localToGlobalModels, localToGlobalAnims, localToGlobalMats);

        return success;
    }

    void ManifestSceneWriter::ProcessModels(Scene& scene, std::vector<std::string>& outModelManifest, std::vector<uint32_t>& outLocalToGlobalModels)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!modelPool) return;

        auto modelManager = ServiceLocator::Get<ModelManager>();
        uint32_t maxModelId = static_cast<uint32_t>(modelManager->GetResourceCount());

        std::vector<uint8_t> usedModels(maxModelId, 0);
        for (auto entity : modelPool->GetStorage().GetDenseEntities())
            usedModels[modelPool->Get(entity).modelIndex] = 1;

        std::vector<std::string> allModelPaths = modelManager->GetResourcePaths();
        for (uint32_t globalIndex = 0; globalIndex < maxModelId; ++globalIndex)
        {
            if (usedModels[globalIndex] == 1 && globalIndex < allModelPaths.size())
            {
                const std::string& resPath = allModelPaths[globalIndex];
                if (!resPath.empty())
                {
                    outModelManifest.push_back(resPath);
                    outLocalToGlobalModels.push_back(globalIndex);
                }
            }
        }

        std::vector<uint32_t> globalToLocal(maxModelId, UINT32_MAX);
        for (uint32_t i = 0; i < outLocalToGlobalModels.size(); ++i)
        {
            globalToLocal[outLocalToGlobalModels[i]] = i;
        }

        for (auto entity : modelPool->GetStorage().GetDenseEntities())
        {
            auto& comp = modelPool->Get(entity);

            if (comp.modelIndex != UINT32_MAX && comp.modelIndex < globalToLocal.size()) {
                comp.modelIndex = globalToLocal[comp.modelIndex];
            }
        }
    }

    void ManifestSceneWriter::ProcessAnimations(Scene& scene, std::vector<AnimationManifestEntry>& outAnimManifest, std::vector<uint32_t>& outLocalToGlobalAnims)
    {
        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!animPool || !modelPool) return;

        auto animManager = ServiceLocator::Get<AnimationManager>();
        uint32_t maxAnimId = animManager->GetResourceCount();

        std::vector<uint8_t> usedAnims(maxAnimId, 0);
        std::vector<uint32_t> animToLocalModel(maxAnimId, UINT32_MAX);
        for (auto entity : animPool->GetStorage().GetDenseEntities())
        {
            uint32_t animIdx = animPool->Get(entity).animationIndex;
            usedAnims[animIdx] = 1;

            if (modelPool->Has(entity))
                animToLocalModel[animIdx] = modelPool->Get(entity).modelIndex;
        }

        std::vector<std::string> allAnimPaths = animManager->GetResourcePaths();
        for (uint32_t globalIndex = 0; globalIndex < maxAnimId; ++globalIndex)
        {
            if (usedAnims[globalIndex] == 1 && globalIndex < allAnimPaths.size())
            {
                std::string fullPath = allAnimPaths[globalIndex];
                if (!fullPath.empty())
                {
                    size_t underscorePos = fullPath.find_last_of('_');
                    std::string cleanPath = fullPath.substr(0, underscorePos);

                    outAnimManifest.push_back({ cleanPath, animToLocalModel[globalIndex] });
                    outLocalToGlobalAnims.push_back(globalIndex);
                }
            }
        }

        std::vector<uint32_t> globalToLocal(maxAnimId, UINT32_MAX);
        for (uint32_t i = 0; i < outLocalToGlobalAnims.size(); ++i)
        {
            globalToLocal[outLocalToGlobalAnims[i]] = i;
        }

        for (auto entity : animPool->GetStorage().GetDenseEntities())
        {
            auto& comp = animPool->Get(entity);

            if (comp.animationIndex != UINT32_MAX && comp.animationIndex < globalToLocal.size()) {
                comp.animationIndex = globalToLocal[comp.animationIndex];
            }
        }
    }

    void ManifestSceneWriter::ProcessMaterials(Scene& scene, std::vector<MaterialManifestEntry>& outMatManifest, std::vector<uint32_t>& outLocalToGlobalMats)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        auto matPool = scene.GetRegistry()->GetPool<MaterialOverrideComponent>();

        auto modelManager = ServiceLocator::Get<ModelManager>();
        auto matManager = ServiceLocator::Get<MaterialManager>();

        uint32_t maxMatId = matManager->GetResourceCount();
        std::vector<uint8_t> usedMats(maxMatId, 0);

        // 1. Collect default materials from Models (if not overridden)
        if (modelPool)
        {
            auto modelSnapshots = modelManager->GetResourceSnapshot();
            std::vector<uint8_t> processedModels(modelSnapshots.size(), 0);

            for (auto entity : modelPool->GetStorage().GetDenseEntities())
            {
                auto& comp = modelPool->Get(entity);
                uint32_t modelIndex = comp.modelIndex;

                if (modelIndex < modelSnapshots.size() && modelSnapshots[modelIndex].resource)
                {
                    if (processedModels[modelIndex] == 1)
                        continue;

                    processedModels[modelIndex] = 1;

                    const auto& defaultMats = modelSnapshots[modelIndex].resource->cpuData.meshMaterialIndices;
                    for (uint32_t matId : defaultMats) {
                        if (matId != UINT32_MAX && matId < maxMatId)
                            usedMats[matId] = 1;
                    }
                }
            }
        }

        // 2. Collect materials from Override components
        if (matPool)
        {
            for (auto entity : matPool->GetStorage().GetDenseEntities())
            {
                auto& comp = matPool->Get(entity);
                for (uint32_t matId : comp.materials) {
                    if (matId != UINT32_MAX && matId < maxMatId)
                        usedMats[matId] = 1;
                }
            }
        }

        // 3. Populate the Material Manifest
        auto allMatPaths = matManager->GetResourcePaths();
        auto allMatSnapshots = matManager->GetResourceSnapshot();
        for (uint32_t globalIndex = 0; globalIndex < maxMatId; ++globalIndex)
        {
            if (usedMats[globalIndex] == 1)
            {
                MaterialManifestEntry entry{};
                entry.name = (globalIndex < allMatPaths.size() && !allMatPaths[globalIndex].empty())
                    ? allMatPaths[globalIndex]
                    : "Material_" + std::to_string(globalIndex);
                entry.path = entry.name;
                entry.material = *allMatSnapshots[globalIndex].resource;
                
                outMatManifest.push_back(entry);
                outLocalToGlobalMats.push_back(globalIndex);
            }
        }

        // 4. Build reverse dictionary (Global -> Local)
        std::vector<uint32_t> globalToLocal(maxMatId, UINT32_MAX);
        for (uint32_t i = 0; i < outLocalToGlobalMats.size(); ++i) {
            globalToLocal[outLocalToGlobalMats[i]] = i;
        }

        // 5. Remap only the Override components to Local indices (Model defaults are stored in the Model itself)
        if (matPool)
        {
            for (auto entity : matPool->GetStorage().GetDenseEntities())
            {
                auto& comp = matPool->Get(entity);
                for (size_t i = 0; i < comp.materials.size(); ++i)
                {
                    if (comp.materials[i] != UINT32_MAX && comp.materials[i] < maxMatId) {
                        comp.materials[i] = globalToLocal[comp.materials[i]];
                    }
                }
            }
        }
    }

    void ManifestSceneWriter::ProcessTextures(std::vector<MaterialManifestEntry>& inOutMatManifest, std::vector<TextureManifestEntry>& outTexManifest, std::vector<uint32_t>& outLocalToGlobalTex)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t maxTexId = imageManager->GetResourceCount();

        std::vector<uint8_t> usedTex(maxTexId, 0);

        auto markUsed = [&](uint32_t texIndex) {
            if (texIndex != UINT32_MAX && texIndex < maxTexId) 
                usedTex[texIndex] = 1;
            };

        for (const auto& matEntry : inOutMatManifest) {
            markUsed(matEntry.material.albedoTexture);
            markUsed(matEntry.material.normalTexture);
            markUsed(matEntry.material.metalnessTexture);
            markUsed(matEntry.material.roughnessTexture);
            markUsed(matEntry.material.metallicRoughnessTexture);
            markUsed(matEntry.material.emissiveTexture);
            markUsed(matEntry.material.ambientOcclusionTexture);
            markUsed(matEntry.material.opacityTexture);
            markUsed(matEntry.material.clearcoatTexture);
            markUsed(matEntry.material.clearcoatRoughnessTexture);
            markUsed(matEntry.material.clearcoatNormalTexture);
            markUsed(matEntry.material.specularTexture);
            markUsed(matEntry.material.specularColorTexture);
            markUsed(matEntry.material.videoTexture);
        }

        std::vector<std::string> allTexPaths = imageManager->GetResourcePaths();
        for (uint32_t globalIndex = 0; globalIndex < maxTexId; ++globalIndex)
        {
            if (usedTex[globalIndex] == 1)
            {
                TextureManifestEntry entry{};
                entry.name = (globalIndex < allTexPaths.size() && !allTexPaths[globalIndex].empty())
                    ? allTexPaths[globalIndex]
                    : "Texture_" + std::to_string(globalIndex);

                entry.payload.path = entry.name;

                outTexManifest.push_back(entry);
                outLocalToGlobalTex.push_back(globalIndex);
            }
        }

        // 3. Build reverse dictionary (Global -> Local)
        std::vector<uint32_t> globalToLocal(maxTexId, UINT32_MAX);
        for (uint32_t i = 0; i < outLocalToGlobalTex.size(); ++i) {
            globalToLocal[outLocalToGlobalTex[i]] = i;
        }

        // 4. Remap the texture IDs INSIDE the MaterialManifest to Local indices!
        auto remapTex = [&](uint32_t& texIndex) {
            if (texIndex != UINT32_MAX && texIndex < maxTexId) texIndex = globalToLocal[texIndex];
            };

        for (auto& matEntry : inOutMatManifest) {
            remapTex(matEntry.material.albedoTexture);
            remapTex(matEntry.material.normalTexture);
            remapTex(matEntry.material.metalnessTexture);
            remapTex(matEntry.material.roughnessTexture);
            remapTex(matEntry.material.metallicRoughnessTexture);
            remapTex(matEntry.material.emissiveTexture);
            remapTex(matEntry.material.ambientOcclusionTexture);
            remapTex(matEntry.material.opacityTexture);
            remapTex(matEntry.material.clearcoatTexture);
            remapTex(matEntry.material.clearcoatRoughnessTexture);
            remapTex(matEntry.material.clearcoatNormalTexture);
            remapTex(matEntry.material.specularTexture);
            remapTex(matEntry.material.specularColorTexture);
            remapTex(matEntry.material.videoTexture);
        }
    }

    void ManifestSceneWriter::RestoreOriginalIndices(Scene& scene, const std::vector<uint32_t>& localToGlobalModels, const std::vector<uint32_t>& localToGlobalAnims, const std::vector<uint32_t>& localToGlobalMats)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (modelPool)
        {
            for (auto entity : modelPool->GetStorage().GetDenseEntities())
            {
                auto& comp = modelPool->Get(entity);

                if (comp.modelIndex != UINT32_MAX && comp.modelIndex < localToGlobalModels.size()) {
                    comp.modelIndex = localToGlobalModels[comp.modelIndex];
                }
            }
        }

        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        if (animPool)
        {
            for (auto entity : animPool->GetStorage().GetDenseEntities())
            {
                auto& comp = animPool->Get(entity);

                if (comp.animationIndex != UINT32_MAX && comp.animationIndex < localToGlobalAnims.size()) {
                    comp.animationIndex = localToGlobalAnims[comp.animationIndex];
                }
            }
        }

        auto matPool = scene.GetRegistry()->GetPool<MaterialOverrideComponent>();
        if (matPool)
        {
            for (auto entity : matPool->GetStorage().GetDenseEntities())
            {
                auto& comp = matPool->Get(entity);
                for (size_t i = 0; i < comp.materials.size(); ++i)
                {
                    uint32_t localIndex = comp.materials[i];
                    if (localIndex != UINT32_MAX && localIndex < localToGlobalMats.size())
                    {
                        comp.materials[i] = localToGlobalMats[localIndex];
                    }
                }
            }
        }
    }

    void ManifestSceneWriter::ProcessVideos(std::vector<MaterialManifestEntry>& inOutMatManifest, std::vector<VideoManifestEntry>& outVideoManifest, std::vector<uint32_t>& outLocalToGlobalVideo)
    {
        auto videoManager = ServiceLocator::Get<VideoManager>();
        uint32_t maxVidId = videoManager->GetResourceCount();

        std::vector<uint8_t> usedVid(maxVidId, 0);

        auto markUsed = [&](uint32_t vidIndex) {
            if (vidIndex != UINT32_MAX && vidIndex < maxVidId)
                usedVid[vidIndex] = 1;
            };

        for (const auto& matEntry : inOutMatManifest) {
            markUsed(matEntry.material.videoTexture);
        }

        std::vector<std::string> allVidPaths = videoManager->GetResourcePaths();
        for (uint32_t globalIndex = 0; globalIndex < maxVidId; ++globalIndex)
        {
            if (usedVid[globalIndex] == 1)
            {
                VideoManifestEntry entry{};
                entry.name = (globalIndex < allVidPaths.size() && !allVidPaths[globalIndex].empty())
                    ? allVidPaths[globalIndex]
                    : "Video_" + std::to_string(globalIndex);

                entry.path = entry.name;

                outVideoManifest.push_back(entry);
                outLocalToGlobalVideo.push_back(globalIndex);
            }
        }

        std::vector<uint32_t> globalToLocal(maxVidId, UINT32_MAX);
        for (uint32_t i = 0; i < outLocalToGlobalVideo.size(); ++i) {
            globalToLocal[outLocalToGlobalVideo[i]] = i;
        }

        auto remapVid = [&](uint32_t& vidIndex) {
            if (vidIndex != UINT32_MAX && vidIndex < maxVidId) vidIndex = globalToLocal[vidIndex];
            };

        for (auto& matEntry : inOutMatManifest) {
            remapVid(matEntry.material.videoTexture);
        }
    }
}