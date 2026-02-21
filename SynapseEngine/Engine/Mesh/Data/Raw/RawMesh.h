#pragma once
#include "Engine/SynApi.h"
#include "../Common/Vertex.h"
#include <string>

namespace Syn 
{
	struct SYN_API RawMesh
	{
		bool hasNormals = false;
		bool hasTangents = false;

		std::string name;
		uint32_t materialIndex;
		std::vector<uint32_t> indices;
		std::vector<Vertex> vertices; 
	};
}


