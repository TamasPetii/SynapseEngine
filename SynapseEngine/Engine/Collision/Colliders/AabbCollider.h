#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API AabbCollider {
		glm::vec3 min;
		glm::vec3 max;
	};
}
