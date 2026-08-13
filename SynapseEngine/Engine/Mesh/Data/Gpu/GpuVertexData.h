// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <vector>

/// <summary>
/// - Vertex Position and Attributes are stored in separate buffers for depth only rendering, and normal rendering.
/// - Vertex Position stores mesh and node index in a packed uint32_t (uint16_t, uint16_t), nodeTransformations are applied in depth only rendering too!
/// - MeshIndex is used to fetch the correct mesh descriptor and material indices.
/// - NodeIndex is used to fetch the correct node transformation (Loaded by assimp hierarchy)
/// - Bitangent is not stored, but calculated in shader using normal and tangent vectors. (Sponza and some models might have bad light calculations??)
/// </summary>

namespace Syn
{
	struct SYN_API GpuVertexPosition
	{
		glm::vec3 position;
		uint32_t packedIndex; // nodeIndex, meshIndex

		void SetMeshAndNodeIndex(uint16_t meshIndex, uint16_t nodeIndex);
	};

	struct SYN_API GpuVertexAttributes {
		glm::vec3 normal;
		float uv_x;
		glm::vec3 tangent;
		float uv_y;
	};

	struct SYN_API GpuVertexData
	{
		std::vector<GpuVertexPosition> vertexPositions;
		std::vector<GpuVertexAttributes> vertexAttributes;
	};
}
