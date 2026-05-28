#pragma once
#include "Engine/Component/Core/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API CapsuleColliderComponent : public Component
    {
		CapsuleColliderComponent();

		float radius;
		float halfHeight;
		glm::vec3 localOffset;
    };

	struct SYN_API CapsuleColliderComponentGPU
	{
		CapsuleColliderComponentGPU(const CapsuleColliderComponent& component, uint32_t entityIndex);

		glm::vec3 localOffset;
		float radius;
		float halfHeight;
		uint32_t entityIndex;
		float _pad1;
		float _pad2;
	};
}