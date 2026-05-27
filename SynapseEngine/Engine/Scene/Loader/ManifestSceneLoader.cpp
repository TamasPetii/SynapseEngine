#include "ManifestSceneLoader.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"

#include "Engine/Component/Components.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Serialization/Schema/Component/ComponentSchemas.h"

#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Image/ImageManager.h"

namespace Syn
{
    bool ManifestSceneLoader::LoadScene(Scene& scene, const std::filesystem::path& path)
    {
        FullSceneSnapshot snapshot{ scene };

        if (!DeserializeSnapshot(path, snapshot))
            return false;

        std::vector<uint32_t> localToGlobalModels;
        std::vector<uint32_t> localToGlobalAnims;
        std::vector<uint32_t> localToGlobalMats;
        std::vector<uint32_t> localToGlobalTex;

        LoadAndMapModels(snapshot.modelManifest, localToGlobalModels);
        LoadAndMapAnimations(snapshot.animationManifest, localToGlobalModels, localToGlobalAnims);
        LoadAndMapTextures(snapshot.textureManifest, localToGlobalTex);
        RemapAndLoadMaterials(snapshot.materialManifest, localToGlobalTex, localToGlobalMats);

        RemapModelComponents(scene, localToGlobalModels);
        RemapAnimationComponents(scene, localToGlobalAnims);
        RemapMaterialComponents(scene, localToGlobalMats);

		WakeUpEntities(scene);

        Info("ManifestSceneLoader: Successfully populated scene from {}", path.string());
        return true;
    }

    bool ManifestSceneLoader::DeserializeSnapshot(const std::filesystem::path& path, FullSceneSnapshot& outSnapshot)
    {
        auto serializer = ServiceLocator::GetSerializer();
        if (!serializer)
        {
            Error("ManifestSceneLoader: Serializer cannot be resolved via ServiceLocator!");
            return false;
        }

        if (!serializer->LoadFromFile(path, outSnapshot))
        {
            Error("ManifestSceneLoader: Critical error during deserialization from file: {}", path.string());
            return false;
        }

        return true;
    }

    void ManifestSceneLoader::LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        outLocalToGlobalModels.reserve(modelManifest.size());

        for (const auto& modelPath : modelManifest)
        {
            uint32_t globalId = modelManager->LoadModelAsync(modelPath);
            outLocalToGlobalModels.push_back(globalId);
        }
    }

    void ManifestSceneLoader::LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims)
    {
        auto animManager = ServiceLocator::GetAnimationManager();
        outLocalToGlobalAnims.reserve(animManifest.size());

        for (const auto& animEntry : animManifest)
        {
            if (animEntry.localModelIndex >= localToGlobalModels.size())
            {
                Warning("ManifestSceneLoader: Animation entry has out-of-bounds localModelIndex ({}), skipping.", animEntry.localModelIndex);
                outLocalToGlobalAnims.push_back(UINT32_MAX);
                continue;
            }

            uint32_t globalModelId = localToGlobalModels[animEntry.localModelIndex];
            uint32_t globalAnimId = animManager->LoadAnimationAsync(animEntry.filePath, globalModelId);
            outLocalToGlobalAnims.push_back(globalAnimId);
        }
    }

    void ManifestSceneLoader::LoadAndMapTextures(const std::vector<TextureManifestEntry>& texManifest, std::vector<uint32_t>& outLocalToGlobalTex)
    {
        auto imageManager = ServiceLocator::GetImageManager();
        outLocalToGlobalTex.reserve(texManifest.size());

        for (const auto& entry : texManifest)
        {
            uint32_t globalId = imageManager->LoadImageAsync(entry.payload.path);
            outLocalToGlobalTex.push_back(globalId);
        }
    }

    void ManifestSceneLoader::RemapAndLoadMaterials(std::vector<MaterialManifestEntry>& matManifest, const std::vector<uint32_t>& localToGlobalTex, std::vector<uint32_t>& outLocalToGlobalMats)
    {
        auto matManager = ServiceLocator::GetMaterialManager();
        outLocalToGlobalMats.reserve(matManifest.size());

        auto applyRemap = [&](uint32_t& localTexIndex) {
            if (localTexIndex != UINT32_MAX) {
                if (localTexIndex < localToGlobalTex.size()) {
                    localTexIndex = localToGlobalTex[localTexIndex];
                }
                else {
                    Error("ManifestSceneLoader: Material has corrupt local texture index: {}", localTexIndex);
                    localTexIndex = UINT32_MAX;
                }
            }
            };

        for (auto& entry : matManifest)
        {
            applyRemap(entry.material.albedoTexture);
            applyRemap(entry.material.normalTexture);
            applyRemap(entry.material.metalnessTexture);
            applyRemap(entry.material.roughnessTexture);
            applyRemap(entry.material.metallicRoughnessTexture);
            applyRemap(entry.material.emissiveTexture);
            applyRemap(entry.material.ambientOcclusionTexture);

            uint32_t globalId = matManager->LoadMaterialDirect(entry.name, entry.material);
            outLocalToGlobalMats.push_back(globalId);
        }
    }

    void ManifestSceneLoader::RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!modelPool) return;

        for (auto entity : modelPool->GetStorage().GetDenseEntities())
        {
            auto& comp = modelPool->Get(entity);

            if (comp.modelIndex != UINT32_MAX)
            {
                if (comp.modelIndex < localToGlobalModels.size())
                {
                    comp.modelIndex = localToGlobalModels[comp.modelIndex];
                }
                else
                {
                    Error("ManifestSceneLoader: ModelComponent on entity {} has corrupt local index: {}", entity, comp.modelIndex);
                }
            }
        }
    }

    void ManifestSceneLoader::RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims)
    {
        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        if (!animPool) return;

        for (auto entity : animPool->GetStorage().GetDenseEntities())
        {
            auto& comp = animPool->Get(entity);

            if (comp.animationIndex != UINT32_MAX)
            {
                if (comp.animationIndex < localToGlobalAnims.size())
                {
                    uint32_t globalAnimId = localToGlobalAnims[comp.animationIndex];
                    if (globalAnimId != UINT32_MAX)
                    {
                        comp.animationIndex = globalAnimId;
                    }
                }
                else
                {
                    Error("ManifestSceneLoader: AnimationComponent on entity {} has corrupt local index: {}", entity, comp.animationIndex);
                }
            } 
        }
    }

    void ManifestSceneLoader::RemapMaterialComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalMats)
    {
        auto matPool = scene.GetRegistry()->GetPool<MaterialOverrideComponent>();
        if (!matPool) return;

        for (auto entity : matPool->GetStorage().GetDenseEntities())
        {
            auto& comp = matPool->Get(entity);
            for (size_t i = 0; i < comp.materials.size(); ++i)
            {
                uint32_t localIndex = comp.materials[i];

                if (localIndex != UINT32_MAX)
                {
                    if (localIndex < localToGlobalMats.size()) {
                        comp.materials[i] = localToGlobalMats[localIndex];
                    }
                    else {
                        Error("ManifestSceneLoader: MaterialOverrideComponent on entity {} has corrupt local index: {}", (uint32_t)entity, localIndex);
                    }
                }
            }
        }
    }

    void ManifestSceneLoader::WakeUpEntities(Scene& scene)
    {
        WakeUpHelper wakeUp;
        wakeUp.registry = scene.GetRegistry();

        wakeUp.Run<TagComponent>();
        wakeUp.Run<CameraComponent>();
        wakeUp.Run<TransformComponent, TRANSFORM_POS_CHANGED, TRANSFORM_ROT_CHANGED, TRANSFORM_SCALE_CHANGED>();
        wakeUp.Run<ModelComponent>();
        wakeUp.Run<MaterialOverrideComponent>();
        wakeUp.Run<DirectionLightComponent>();
        wakeUp.Run<DirectionLightShadowComponent>();
        wakeUp.Run<PointLightComponent>();
        wakeUp.Run<PointLightShadowComponent>();
        wakeUp.Run<SpotLightComponent>();
        wakeUp.Run<SpotLightShadowComponent>();
        wakeUp.Run<BoxColliderComponent>();
        wakeUp.Run<SphereColliderComponent>();
        wakeUp.Run<CapsuleColliderComponent>();

        wakeUp.Run<AnimationComponent>([](auto& comp, EntityID entity) {
                comp.isReady = false;
                comp.frameIndex = 0;
            });

        wakeUp.Run<RigidBodyComponent>([](auto& comp, EntityID entity) {
            comp.bodyID = INVALID_BODY_ID;
            });

    }
}