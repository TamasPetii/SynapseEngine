#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <glm/glm.hpp>

/// <summary>
/// - MeshData: Index buffer is batched, all lod level's indices are stored in a single buffer
/// - MeshDescriptor: Contains information about a single mesh, with a single material. (Assimp cuts meshes by materials!) This is used to match the vertex meshIndex to materialIndex.
///                   Also MeshDescriptors are stored in a batched buffer, so each lod level will have its only MeshDescriptor (This is actually not necessery and redundant for normal rendering, but its simplier and nicer, better for debug) 
/// - MeshLodDescriptor: Contains information about a single lod level, with multiple meshes. This is used to match the distance to the correct lod level, and fetch the correct mesh descriptors for that lod level. 
///                      This is necessary for gpu-driven rendering, also this tells us the index ranges of Lods in the global batched index buffer
/// </summary>

namespace Syn
{
	struct SYN_API GpuMeshCollider
	{
		glm::vec3 center;
		float radius;
		glm::vec3 aabbMin;
		float padding0;
		glm::vec3 aabbMax;
		float padding1;
	};

	struct SYN_API GpuMeshDescriptor
	{
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t materialIndex;
	};

	struct SYN_API GpuMeshLodDescriptor
	{
		uint32_t meshOffset;
		uint32_t meshCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		float distanceThreshold;
	};

	struct SYN_API GpuIndexedDrawData
	{
		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<uint32_t> indices;

		//New: [(Mesh0_Lod0, Mesh0_Lod1, Mesh0_Lod2, Mesh0_Lod3), (Mesh1_Lod0, ...)]
		std::vector<GpuMeshDescriptor> meshDescriptors;

		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<GpuMeshLodDescriptor> lodDescriptors;

		std::vector<GpuMeshCollider> meshColliders;
	};
}

