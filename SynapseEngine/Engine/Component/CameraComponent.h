#pragma once
#include "BaseComponent/Component.h"
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


