// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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