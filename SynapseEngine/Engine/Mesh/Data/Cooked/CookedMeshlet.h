#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "CookedColliders.h"

namespace Syn
{
	struct SYN_API CookedMeshlet
	{
		uint32_t vertexOffset;
		uint32_t triangleOffset;

		uint32_t vertexCount;
		uint32_t triangleCount;

		CookedMeshletCollisionData collider;
	};
}


