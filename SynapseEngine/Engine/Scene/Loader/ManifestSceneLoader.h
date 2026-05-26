#pragma once
#include "ISceneLoader.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSnapshotTypes.h"
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API ManifestSceneLoader : public ISceneLoader
    {
    public:
        ManifestSceneLoader() = default;
        ~ManifestSceneLoader() override = default;

        bool LoadScene(Scene& scene, const std::filesystem::path& path) override;
    private:
        bool DeserializeSnapshot(const std::filesystem::path& path, FullSceneSnapshot& outSnapshot);
        void LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels);
        void LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims);
        void RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels);
        void RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims);
    };
}