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

#include "DefaultModelCooker.h"

namespace Syn
{
    CookedMesh DefaultModelCooker::CookMesh(RawMesh&& rawMesh)
    {
        CookedMesh cookedMesh;
        cookedMesh.name = std::move(rawMesh.name);
        cookedMesh.hasNormals = rawMesh.hasNormals;
        cookedMesh.hasTangents = rawMesh.hasTangents;
        cookedMesh.materialIndex = rawMesh.materialIndex;
        cookedMesh.vertices = std::move(rawMesh.vertices);

        CookedMeshLod lod0;
        lod0.indices = std::move(rawMesh.indices);
        cookedMesh.lods.push_back(std::move(lod0));

        return cookedMesh;
    }

    CookedModel DefaultModelCooker::Cook(RawModel&& rawModel)
    {
        CookedModel cookedModel;
        cookedModel.meshes.reserve(rawModel.meshes.size());

        for (auto& rawMesh : rawModel.meshes) {
            cookedModel.meshes.push_back(CookMesh(std::move(rawMesh)));
        }

        cookedModel.materials = std::move(rawModel.materials);
        cookedModel.nodeTransforms = std::move(rawModel.nodeTransforms);
        cookedModel.meshNodeDescriptors = std::move(rawModel.meshNodeDescriptors);

        return cookedModel;
    }
}