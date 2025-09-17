#include "Light.h"

LightShadow::LightShadow() :
	use(false),
	textureSize(2048),
	updateFrequency(1)
{
}

Light::Light() : 
	color(glm::vec3(1.f)),
	strength(1.f),
	shininess(32),
	falloff(LINEAR)
{
}
