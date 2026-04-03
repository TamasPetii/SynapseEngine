#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API GpuNodeTransform
	{
		glm::mat4 transform;
		glm::mat4 transformIT;
	};
}


