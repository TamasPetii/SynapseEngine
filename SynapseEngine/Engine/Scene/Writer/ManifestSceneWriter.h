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
        void ProcessMaterials(Scene& scene, std::vector<MaterialManifestEntry>& outMatManifest, std::vector<uint32_t>& outLocalToGlobalMats);
        void ProcessTextures(std::vector<MaterialManifestEntry>& inOutMatManifest, std::vector<TextureManifestEntry>& outTexManifest, std::vector<uint32_t>& outLocalToGlobalTex);
        void RestoreOriginalIndices(Scene& scene, const std::vector<uint32_t>& localToGlobalModels, const std::vector<uint32_t>& localToGlobalAnims, const std::vector<uint32_t>& localToGlobalMats);
        void ProcessVideos(std::vector<MaterialManifestEntry>& inOutMatManifest, std::vector<VideoManifestEntry>& outVideoManifest, std::vector<uint32_t>& outLocalToGlobalVideo);
    };
}