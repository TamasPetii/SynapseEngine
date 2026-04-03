#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <vector>

namespace Syn
{
	struct SYN_API Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec2 uv;
	};
}


