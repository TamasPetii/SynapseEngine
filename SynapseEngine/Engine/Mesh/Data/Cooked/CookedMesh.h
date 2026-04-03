#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "CookedMeshlet.h"
#include "../Common/Vertex.h"

namespace Syn
{
	struct SYN_API CookedMeshLod {
		//Normal pipeline
		std::vector<uint32_t> indices;

		//Meshlet pipeline
		std::vector<CookedMeshlet> meshlets;
		std::vector<uint32_t> meshletVertexIndices;
		std::vector<uint8_t> meshletTriangleIndices;
	};

	struct SYN_API CookedMesh
	{
		bool hasNormals = false;
		bool hasTangents = false;

		std::string name;
		uint32_t materialIndex;

		std::vector<Vertex> vertices;
		std::vector<CookedMeshLod> lods;
		CookedMeshCollisionData collider;
	};
}


