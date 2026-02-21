#pragma once
#include "Engine/SynApi.h"
#include "VertexData.h"

/// <summary>
/// - Vertex Position and Attributes are stored in separate buffers for depth only rendering, and normal rendering.
/// </summary>

namespace Syn
{
	struct SYN_API GeometryData
	{
		std::vector<VertexPosition> vertexPositions;
		std::vector<VertexAttributes> vertexAttributes;
		//Todo: Global model AABB?
		//Todo: Global model Sphere Collider?
	};
}

