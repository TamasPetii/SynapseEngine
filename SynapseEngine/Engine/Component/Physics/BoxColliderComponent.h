#pragma once
#include "Engine/Component/Core/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API BoxColliderComponent : public Component
    {
		BoxColliderComponent();

        glm::vec3 halfExtents;
        glm::vec3 localOffset;
    };

	struct SYN_API BoxColliderComponentGPU
	{
		BoxColliderComponentGPU(const BoxColliderComponent& component);

		glm::vec3 halfExtents;
		float padding0;
		glm::vec3 localOffset;
		float padding1;
	};
}