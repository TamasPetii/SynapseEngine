#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API ConeCollider
	{
		glm::vec3 apex;
		glm::vec3 axis;
		float cutoff;
	};
}
