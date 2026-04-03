#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API SphereCollider
	{
		glm::vec3 center;
		float radius;
	};
}