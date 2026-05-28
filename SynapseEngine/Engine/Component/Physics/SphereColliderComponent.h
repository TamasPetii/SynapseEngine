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
		SphereColliderComponentGPU(const SphereColliderComponent& component, uint32_t entityIndex);

		glm::vec3 localOffset;
		float radius;
		uint32_t entityIndex;
		float pad0;
		float pad1;
		float pad2;
	};
}