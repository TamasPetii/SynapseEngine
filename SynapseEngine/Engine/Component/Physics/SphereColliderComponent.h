#pragma once
#include "Engine/Component/Core/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API SphereColliderComponent : public Component
    {
		SphereColliderComponent();

		float radius;
		glm::vec3 localOffset;
    };

	struct SYN_API SphereColliderComponentGPU
	{
		SphereColliderComponentGPU(const SphereColliderComponent& component);

		glm::vec3 localOffset;
		float radius;
	};
}