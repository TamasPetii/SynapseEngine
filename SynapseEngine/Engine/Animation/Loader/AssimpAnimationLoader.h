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
#include "IAnimationLoader.h"
#include <assimp/scene.h>

#include <chrono>
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
        void ProcessBoneWeights(const aiScene* scene, RawAnimation& outAnim, const std::unordered_map<std::string, uint32_t>& nodeNameMap, tf::Taskflow& taskflow);
        void ProcessAnimationTracks(const aiScene* scene, RawAnimation& outAnim, const std::unordered_map<std::string, uint32_t>& nodeNameMap, tf::Taskflow& taskflow);
    };
}