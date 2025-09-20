#pragma once
#include "BaseComponents/Component.h"
#include "BaseComponents/Light.h"
#include "BaseComponents/VersionIndexed.h"

#include <glm/glm.hpp>
#include <memory>
#include <array>

constexpr glm::vec3 defaultDirectionLightDirection = glm::vec3(0.f, -1.f, 0.f);

class DirectionLightSystem;

struct ENGINE_API DirectionLightShadow : public LightShadow
{
	DirectionLightShadow();
	std::array<float, 5> farPlane;
	std::array<glm::mat4, 4> viewProj;
	//Maybe dynamic? Or max 6???
};

struct ENGINE_API DirectionLightComponent : public Light, public Component
{
	DirectionLightComponent();

	glm::vec3 direction; //Mapped to transform component rotation!
	DirectionLightShadow shadow;
private:
	glm::vec3 position;
	friend class DirectionLightSystem;
};

struct ENGINE_API DirectionLightGPU
{
	DirectionLightGPU(const DirectionLightComponent& directionLightComponent);

	glm::vec3 color;
	float strength;
	glm::vec3 direction;
	float shininess;
	//Todo: bitflag
	//Todo: shadow viewProjs, farPlanes
};

