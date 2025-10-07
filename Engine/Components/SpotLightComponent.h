#pragma once
#include "BaseComponents/Component.h"
#include "BaseComponents/Light.h"
#include "BaseComponents/VersionIndexed.h"
#include "BaseComponents/FrustumCullable.h"

#include <glm/glm.hpp>
#include <memory>
#include <array>

constexpr glm::vec3 defaultSpotLightDirection = glm::vec3(1.f, 0.f, 0.f);

class SpotLightSystem;

struct ENGINE_API SpotLightShadow : public LightShadow
{
	SpotLightShadow();

	//farplane = length;
	glm::mat4 viewProj;
};

struct ENGINE_API SpotLightComponent : public Light, public Component, public FrustumCullable
{
	SpotLightComponent();

	glm::vec3 position; //Mapped to transform component translation!
	glm::vec3 direction; //Mapped to transform component rotation!
	glm::vec4 angles; //x = inner bound to transform component scale x | y = outer angle bound to transform component scale y | zw = cos(angles)
	float range; //Mapped to transform component scale z
	SpotLightShadow shadow;
	//Todo: Visible entities??? Instanced???? Maybe in models???? Or like a model map?
private:
	glm::vec3 boundingSphereOrigin;
	float boundingSphereRadius;
	glm::vec3 aabbOrigin;
	glm::vec3 aabbExtents;
	glm::mat4 transform; //Cannot use transform component becouse of the scale can be different on xyz!
	friend class SpotLightSystem;
	friend struct SpotLightGPU;
};

struct ENGINE_API SpotLightGPU
{
	SpotLightGPU(const SpotLightComponent& spotLightComponent);

	glm::vec3 color;
	float strength;
	glm::vec3 position;
	float shininess;
	glm::vec3 direction;
	float range;
	glm::vec4 angles; //xy = inner/outer | zw = cos(inner)/cos(outer)
	glm::vec3 boundingSphereOrigin;
	float boundingSphereRadius;
	glm::vec3 aabbOrigin;
	uint32_t bitflag;
	glm::vec3 aabbExtents;
	float padding; //Padding for 16 byte alignment
};