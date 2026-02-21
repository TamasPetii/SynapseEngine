#include "CookedUtils.h"

namespace Syn
{
	CookedMesh CookedUtils::ConvertRawMesh(RawMesh&& rawMesh)
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

    CookedModel CookedUtils::ConvertRawModel(RawModel&& rawModel)
    {
        CookedModel cookedModel;
        cookedModel.meshes.reserve(rawModel.meshes.size());

        for (auto& rawMesh : rawModel.meshes)
            cookedModel.meshes.push_back(ConvertRawMesh(std::move(rawMesh)));

        cookedModel.materialNames = std::move(rawModel.materialNames);
        cookedModel.nodeTransforms = std::move(rawModel.nodeTransforms);
        cookedModel.meshNodeDescriptors = std::move(rawModel.meshNodeDescriptors);

        return cookedModel;
    }
}