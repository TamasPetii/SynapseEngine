#pragma once
#include "Engine/SynApi.h"
#include "IAnimationLoader.h"

namespace tinygltf {
    class Model;
}

namespace tf {
    class Taskflow;
}

namespace Syn
{
    class SYN_API TinyGltfAnimationLoader : public IAnimationLoader
    {
    public:
        TinyGltfAnimationLoader() = default;
        virtual std::optional<RawAnimation> LoadFile(const std::filesystem::path& path) override;
        virtual std::vector<std::string> GetSupportedExtensions() const override;

    private:
        void BuildLinearHierarchy(const tinygltf::Model& gltfModel, RawAnimation& outAnim, std::vector<uint32_t>& outGltfToFlatNode);
        void ProcessAnimationTracks(const tinygltf::Model& gltfModel, RawAnimation& outAnim, const std::vector<uint32_t>& gltfToFlatNode, tf::Taskflow& taskflow);
        void ProcessBoneWeights(const tinygltf::Model& gltfModel, RawAnimation& outAnim, const std::vector<uint32_t>& gltfToFlatNode, tf::Taskflow& taskflow);

    private:
        std::vector<std::vector<uint32_t>> _meshPrimitiveToRawMeshIndex;
        uint32_t _totalPrimitives = 0;
    };
}