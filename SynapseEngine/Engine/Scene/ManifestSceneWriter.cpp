#include "ManifestSceneWriter.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Component/Rendering/AnimationComponent.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn
{
    bool ManifestSceneWriter::SaveScene(Scene& scene, const std::filesystem::path& path)
    {
        std::vector<std::string> modelManifest;
        std::vector<uint32_t> localToGlobalModels;

        std::vector<AnimationManifestEntry> animManifest;
        std::vector<uint32_t> localToGlobalAnims;

        ProcessModels(scene, modelManifest, localToGlobalModels);
        ProcessAnimations(scene, animManifest, localToGlobalAnims);

        using FullSceneSnapshot = SceneSnapshot<TransformComponent, ModelComponent, AnimationComponent>;
        FullSceneSnapshot snapshot{ scene };
        snapshot.modelManifest = std::move(modelManifest);
        snapshot.animationManifest = std::move(animManifest);

        auto serializer = ServiceLocator::GetSerializer();
        bool success = false;
        if (serializer)
            success = serializer->SaveToFile(path, snapshot);

        RestoreOriginalIndices(scene, localToGlobalModels, localToGlobalAnims);

        return success;
    }

    void ManifestSceneWriter::ProcessModels(Scene& scene, std::vector<std::string>& outModelManifest, std::vector<uint32_t>& outLocalToGlobalModels)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!modelPool) return;

        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t maxModelId = modelManager->GetResourceCount();

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
            comp.modelIndex = globalToLocal[comp.modelIndex];
        }
    }

    void ManifestSceneWriter::ProcessAnimations(Scene& scene, std::vector<AnimationManifestEntry>& outAnimManifest, std::vector<uint32_t>& outLocalToGlobalAnims)
    {
        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!animPool || !modelPool) return;

        auto animManager = ServiceLocator::GetAnimationManager();
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
            comp.animationIndex = globalToLocal[comp.animationIndex];
        }
    }

    void ManifestSceneWriter::RestoreOriginalIndices(Scene& scene, const std::vector<uint32_t>& localToGlobalModels, const std::vector<uint32_t>& localToGlobalAnims)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (modelPool)
        {
            for (auto entity : modelPool->GetStorage().GetDenseEntities())
            {
                auto& comp = modelPool->Get(entity);
                comp.modelIndex = localToGlobalModels[comp.modelIndex];
            }
        }

        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        if (animPool)
        {
            for (auto entity : animPool->GetStorage().GetDenseEntities())
            {
                auto& comp = animPool->Get(entity);
                comp.animationIndex = localToGlobalAnims[comp.animationIndex];
            }
        }
    }
}