// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/Component/Core/Component.h"
#include "Engine/Collision/Colliders/FrustumCollider.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API CameraComponent : public Component
	{
		CameraComponent();

		float yaw;
		float pitch;

		float nearPlane;
		float farPlane;

		float fov;
		float width;
		float height;
		float speed;
		float sensitivity;
		float distance;

		bool isOrthographic;
		float orthoSize;
		bool useOrbit;

		glm::vec3 up;
		glm::vec3 target;
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 right;

		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 viewProj;
		glm::mat4 viewProjInv;

		FrustumCollider frustum;
	};

	struct SYN_API CameraComponentGPU
	{
		CameraComponentGPU(const CameraComponent& component);

		glm::mat4 view;
		glm::mat4 viewInv;
		glm::mat4 proj;
		glm::mat4 projInv;
		glm::mat4 projVulkan;
		glm::mat4 projVulkanInv;
		glm::mat4 viewProj;
		glm::mat4 viewProjInv;
		glm::mat4 viewProjVulkan;
		glm::mat4 viewProjVulkanInv;
		glm::vec4 eye;
		glm::vec4 params;
		glm::vec4 frustum[6];
	};
}


