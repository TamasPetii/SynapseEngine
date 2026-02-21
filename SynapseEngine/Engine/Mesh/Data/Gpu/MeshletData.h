#pragma once
#include "Engine/SynApi.h"
#include "VertexData.h"
#include <vector>

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
	struct SYN_API MeshletDescriptor
	{
		uint32_t vertexIndicesOffset;
		uint8_t vertexCount;
		uint32_t triangleIndicesOffset;
		uint8_t triangleCount;

		// Local meshlet bounding sphere
		glm::vec3 center;
		float radius;

		//Local meshlet bounding cone
		glm::vec3 coneAxis;
		float coneCutoff;

		//Todo: Local meshlet AABB?
	};

	struct SYN_API MeshletLodDescriptor
	{
		uint32_t vertexIndexOffset;
		uint32_t vertexIndexCount;
		uint32_t triangleIndexOffset;
		uint32_t triangleIndexCount;
		uint32_t meshletOffset;
		uint32_t meshletCount;
		float distanceThreshold;
		//Todo: Global Lod meshlet AABB?
		//Todo: Global Lod meshlet Sphere Collider?
	};

	struct SYN_API MeshletData
	{
		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<uint32_t> vertexIndices;

		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<uint8_t>  triangleIndices;

		//[(Lod0 - Meshlet0, ..., MeshletN) | ... | (Lod3 - Meshlet0, ..., MeshletN)]
		std::vector<MeshletDescriptor> descriptors;

		//[Lod0 | Lod1 | Lod2 | Lod3]
		std::vector<MeshletLodDescriptor> lods;
	};
}

