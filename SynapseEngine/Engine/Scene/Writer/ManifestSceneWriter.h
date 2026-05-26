#pragma once
#include "ISceneWriter.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API ManifestSceneWriter : public ISceneWriter
    {
    public:
        ManifestSceneWriter() = default;
        ~ManifestSceneWriter() override = default;

        bool SaveScene(Scene& scene, const std::filesystem::path& path) override;
    private:
        void ProcessModels(Scene& scene, std::vector<std::string>& outModelManifest, std::vector<uint32_t>& outLocalToGlobalModels);
        void ProcessAnimations(Scene& scene, std::vector<AnimationManifestEntry>& outAnimManifest, std::vector<uint32_t>& outLocalToGlobalAnims);
        void RestoreOriginalIndices(Scene& scene, const std::vector<uint32_t>& localToGlobalModels, const std::vector<uint32_t>& localToGlobalAnims);
    };
}