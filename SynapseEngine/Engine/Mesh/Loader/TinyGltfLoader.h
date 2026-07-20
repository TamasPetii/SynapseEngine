#pragma once
#include "Engine/SynApi.h"
#include "IMeshLoader.h"
#include <filesystem>
#include <vector>
#include <optional>

namespace tinygltf {
    class Model;
}

namespace tf {
    class Taskflow;
}

namespace Syn
{
    class SYN_API TinyGltfLoader : public IMeshLoader
    {
    public:
        TinyGltfLoader() = default;
        virtual std::optional<RawModel> LoadFile(const std::filesystem::path& path) override;
        virtual std::vector<std::string> GetSupportedExtensions() const override;
    private:
        void ProcessMaterials(const tinygltf::Model& gltfModel, const std::filesystem::path& basePath, RawModel& outModel, tf::Taskflow& taskflow);
        void ProcessMeshes(const tinygltf::Model& gltfModel, RawModel& outModel, tf::Taskflow& taskflow);
        void ProcessNodes(const tinygltf::Model& gltfModel, RawModel& outModel);
    private:
        std::vector<std::vector<uint32_t>> _meshPrimitiveToRawMeshIndex;
    };
}