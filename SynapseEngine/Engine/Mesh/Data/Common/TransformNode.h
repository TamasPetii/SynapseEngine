#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API TransformNode
	{
		glm::mat4 localTransform = glm::mat4(1.0f);
		glm::mat4 globalTransform = glm::mat4(1.0f);
		glm::mat4 globalTransformIT = glm::mat4(1.0f);
	};
}


