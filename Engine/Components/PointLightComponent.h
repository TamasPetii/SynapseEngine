#pragma once
#include "BaseComponents/Component.h"
#include "BaseComponents/Light.h"
#include "BaseComponents/VersionIndexed.h"
#include "BaseComponents/FrustumCullable.h"
#include "Engine/Renderable/Instanceable.h"

#include <glm/glm.hpp>
#include <memory>
#include <array>

constexpr float defaultPointLightRadius = 1.f;

class PointLightSystem;

struct ENGINE_API PointLightShadow : public LightShadow
{
	PointLightShadow();
	//farplane = radius
	std::array<glm::mat4, 6> viewProj;
};

struct ENGINE_API PointLightComponent : public Light, public Component, public FrustumCullable
{
	PointLightComponent();
	static inline uint32_t instanceCount = 0; //Probably problematic with more scenes on the fly
	static inline std::vector<uint32_t> instanceIndices;

	glm::vec3 position; //Mapped to transform component translation!
	float radius; //Mapped to transform component scale (max value)
	float weakenDistance;
	PointLightShadow shadow;
	//Todo: Visible entities??? Instanced???? Maybe in models???? Or like a model map?
private:
	glm::mat4 transform; //Cannot use transform component becouse of the scale can be different on xyz!
	friend class PointLightSystem;
};

struct ENGINE_API PointLightGPU
{
	PointLightGPU(const PointLightComponent& pointLightComponent);

	glm::vec3 color;
	float strength;
	glm::vec3 position;
	float shininess;
	float radius;
	float weakenDistance;
	uint32_t bitflag;
	uint32_t padding;
};