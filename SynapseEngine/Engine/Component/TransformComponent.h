#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API TransformComponent : public Component
	{
		TransformComponent();

		glm::vec3 translation;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 transform;
		glm::mat4 transformIT;
	};

	struct SYN_API TransformComponentGPU
	{
		TransformComponentGPU(const TransformComponent& component);

		glm::mat4 transform;
		glm::mat4 transformIT;
	};
}