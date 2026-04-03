#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <glm/glm.hpp>

/// <summary>
/// - MeshletData: Similar to MeshData, but for meshlets. MeshletDescriptor contains information about a single meshlet, and MeshletLodDescriptor contains information about a single lod level of meshlets. 
///                This is used for gpu-driven rendering with meshlets.
///                VertexIndices and TriangleIndices are stored in separate buffers, and MeshletDescriptor contains the offsets and counts for those buffers. 
///                This is because meshlets have a maximum vertex count of 64 and triangle count of 126, which can be easily stored in uint8_t, and we can save memory by using uint8_t instead of uint32_t for indices.
///                VertexIndices are used to index into the global vertex buffer, and triangle indices are used to build triangles of the vertex indices.
/// - MeshletDescriptor: Contains information about a single meshlet, including its vertex and triangle indices offsets and counts, as well as its bounding sphere and cone for culling purposes.
///                      All lod level meshlet descriptors are stored in a single flat buffer.
/// - MeshletLodDescriptor: Contains information about a single lod level of meshlets, including the offsets and counts for vertex indices, triangle indices, and meshlet descriptors, as well as the distance threshold for this lod level.
/// </summary>

namespace Syn
{
	struct SYN_API GpuMeshletDescriptor
	{
		uint32_t vertexIndicesOffset;
		uint32_t vertexCount;
		uint32_t triangleIndicesOffset;
		uint32_t triangleCount;
	};

	struct SYN_API GpuMeshletCollider
	{
		glm::vec3 center;
		float radius;
		glm::vec3 aabbMin;
		float padding0;
		glm::vec3 aabbMax;
		float padding1;
		glm::vec3 apex;
		float cutoff;
		glm::vec3 axis;
		float padding2;
	};

	struct SYN_API GpuMeshletDrawDescriptor {
		uint32_t meshletOffset;
		uint32_t meshletCount;
		uint32_t materialIndex;
		uint32_t padding; 
	};

	struct SYN_API GpuMeshletDrawData
	{
		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<uint32_t> vertexIndices;

		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<uint8_t> triangleIndices;

		//New: [(Mesh0_Lod0, Mesh0_Lod1, Mesh0_Lod2, Mesh0_Lod3), (Mesh1_Lod0, ...)] 
		std::vector<GpuMeshletDescriptor> meshletDescriptors;

		//New: [(Mesh0_Lod0, Mesh0_Lod1, Mesh0_Lod2, Mesh0_Lod3), (Mesh1_Lod0, ...)] 
		std::vector<GpuMeshletDrawDescriptor> drawDescriptors;

		std::vector<GpuMeshletCollider> meshletColliders;
	};
}

