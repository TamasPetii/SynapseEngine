#pragma once
#include "Engine/SynApi.h"
#include "RawVertex.h"
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
		std::vector<RawVertex> vertices; 
	};
}


