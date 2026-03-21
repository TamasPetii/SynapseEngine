#pragma once
#include "Engine/SynApi.h"
#include "IAnimationLoader.h"
#include <assimp/scene.h>
#include <taskflow/taskflow.hpp>

namespace Syn
{
    class SYN_API AssimpAnimationLoader : public IAnimationLoader
    {
    public:
        AssimpAnimationLoader() = default;
        virtual std::optional<RawAnimation> LoadFile(const std::filesystem::path& path) override;
        virtual std::vector<std::string> GetSupportedExtensions() const override;
    private:
        void BuildLinearHierarchy(const aiScene* scene, RawAnimation& outAnim);
        void ProcessBoneWeights(const aiScene* scene, RawAnimation& outAnim, tf::Taskflow& taskflow);
        void ProcessAnimationTracks(const aiScene* scene, RawAnimation& outAnim, tf::Taskflow& taskflow);
    };
}