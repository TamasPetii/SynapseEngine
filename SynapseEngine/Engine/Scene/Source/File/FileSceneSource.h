#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Source/ISceneSource.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSnapshotTypes.h"
#include <filesystem>
namespace Syn
{
    class SYN_API FileSceneSource : public ISceneSource
    {
    public:
        FileSceneSource(std::filesystem::path path);
        virtual bool Populate(Scene& scene) override;
    private:
    private:
        bool DeserializeSnapshot(FullSceneSnapshot& outSnapshot);
        void LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels);
        void LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims);
        void RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels);
        void RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims);
    private:
        std::filesystem::path _path;
    };
}