#include "FileSceneSource.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"

#include "Engine/Component/Components.h"

#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Core/GlmSchema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include "Engine/Serialization/Schema/Component/ComponentSchemas.h"

#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"

namespace Syn
{

    FileSceneSource::FileSceneSource(std::filesystem::path path)
        : _path(std::move(path))
    {}

    bool FileSceneSource::Populate(Scene& scene)
    {
        FullSceneSnapshot snapshot{ scene };

        if (!DeserializeSnapshot(snapshot))
            return false;

        std::vector<uint32_t> localToGlobalModels;
        std::vector<uint32_t> localToGlobalAnims;

        LoadAndMapModels(snapshot.modelManifest, localToGlobalModels);
        LoadAndMapAnimations(snapshot.animationManifest, localToGlobalModels, localToGlobalAnims);

        RemapModelComponents(scene, localToGlobalModels);
        RemapAnimationComponents(scene, localToGlobalAnims);

        Info("Successfully populated scene from hierarchy-split source: {}", _path.string());
        return true;
    }

    bool FileSceneSource::DeserializeSnapshot(FullSceneSnapshot& outSnapshot)
    {
        auto serializer = ServiceLocator::GetSerializer();
        if (!serializer)
        {
            Error("FileSceneSource: Serializer cannot be resolved via ServiceLocator!");
            return false;
        }

        if (!serializer->LoadFromFile(_path, outSnapshot))
        {
            Error("FileSceneSource: Critical error during deserialization from file: {}", _path.string());
            return false;
        }

        return true;
    }

    void FileSceneSource::LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        outLocalToGlobalModels.reserve(modelManifest.size());

        for (const auto& modelPath : modelManifest)
        {
            uint32_t globalId = modelManager->LoadModelSync(modelPath);
            outLocalToGlobalModels.push_back(globalId);
        }
    }

    void FileSceneSource::RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels)
    {
        auto modelPool = scene.GetRegistry()->GetPool<ModelComponent>();
        if (!modelPool) return;

        for (auto entity : modelPool->GetStorage().GetDenseEntities())
        {
            auto& comp = modelPool->Get(entity);

            if (comp.modelIndex < localToGlobalModels.size())
            {
                comp.modelIndex = localToGlobalModels[comp.modelIndex];
            }
            else
            {
                Error("FileSceneSource: ModelComponent on entity {} has corrupt local index: {}", entity, comp.modelIndex);
            }
        }
    }

    void FileSceneSource::LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims)
    {
        auto animManager = ServiceLocator::GetAnimationManager();
        outLocalToGlobalAnims.reserve(animManifest.size());

        for (const auto& animEntry : animManifest)
        {
            if (animEntry.localModelIndex >= localToGlobalModels.size())
            {
                Warning("FileSceneSource: Animation entry has out-of-bounds localModelIndex ({}), skipping.", animEntry.localModelIndex);
                outLocalToGlobalAnims.push_back(UINT32_MAX);
                continue;
            }
            uint32_t globalModelId = localToGlobalModels[animEntry.localModelIndex];
            uint32_t globalAnimId = animManager->LoadAnimationSync(animEntry.filePath, globalModelId);
            outLocalToGlobalAnims.push_back(globalAnimId);
        }
    }

    void FileSceneSource::RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims)
    {
        auto animPool = scene.GetRegistry()->GetPool<AnimationComponent>();
        if (!animPool) return;

        for (auto entity : animPool->GetStorage().GetDenseEntities())
        {
            auto& comp = animPool->Get(entity);

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
                Error("FileSceneSource: AnimationComponent on entity {} has corrupt local index: {}", entity, comp.animationIndex);
            }
        }
    }
}