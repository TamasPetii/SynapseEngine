#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "Engine/Registry/BitFlag.h"
#include <glm/glm.hpp>

namespace Syn
{
	constexpr uint32_t TRANSFORM_POS_CHANGED = CUSTOM_CHANGED_BIT1;
	constexpr uint32_t TRANSFORM_ROT_CHANGED = CUSTOM_CHANGED_BIT2;
	constexpr uint32_t TRANSFORM_SCALE_CHANGED = CUSTOM_CHANGED_BIT3;

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