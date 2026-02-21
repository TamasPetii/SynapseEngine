#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
	struct SYN_API RawMeshNodeDescriptor
	{
		uint16_t meshIndex;
		uint16_t nodeIndex;
		uint16_t parentNodeIndex;
		uint32_t vertexCount;
		uint32_t indexCount;
	};
}


