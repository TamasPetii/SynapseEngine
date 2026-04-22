#include "CameraComponent.h"

namespace Syn
{
	CameraComponent::CameraComponent()
		: speed{ 10.f }
		, sensitivity{ 0.3f }
		, width{ 1920 }
		, height{ 1080 }
		, fov{ 60.f }
		, nearPlane{ 0.01f }
		, farPlane{ 1000.f }
		, distance{ 5.f }
		, position{ 0.f, 0.f, 0.f }
		, target{ 0.f, 0.f, -1.f }
		, up{ 0.f, 1.f, 0.f }
	{
		direction = glm::normalize(target - position);
		right = glm::normalize(glm::cross(direction, up));
		yaw = glm::degrees(atan2f(direction.z, direction.x));
		pitch = glm::degrees(asinf(direction.y));

		view = glm::mat4(1.f);
		viewInv = glm::mat4(1.f);
		proj = glm::mat4(1.f);
		projInv = glm::mat4(1.f);
		viewProj = glm::mat4(1.f);
		viewProjInv = glm::mat4(1.f);
	}

	CameraComponentGPU::CameraComponentGPU(const CameraComponent& component) :
		view(component.view),
		viewInv(component.viewInv),
		proj(component.proj),
		projInv(component.projInv),
		viewProj(component.viewProj),
		viewProjInv(component.viewProjInv),
		eye(component.position, 0.f),
		params(component.nearPlane, component.farPlane, component.fov, component.width / component.height)
	{
		this->projVulkan = component.proj;
		this->projVulkan[1][1] *= -1;

		this->projVulkanInv = glm::inverse(this->projVulkan);
		this->viewProjVulkan = this->projVulkan * this->view;
		this->viewProjVulkanInv = glm::inverse(this->viewProjVulkan);
		std::memcpy(frustum, component.frustum.planes, 6 * sizeof(glm::vec4));
	}
}


