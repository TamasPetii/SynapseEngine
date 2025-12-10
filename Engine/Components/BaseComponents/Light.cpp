#include "Light.h"

LightShadow::LightShadow() :
	textureSize(1024),
	updateFrequency(1)
{
}

Light::Light() : 
	useShadow(false),
	color(glm::vec3(1.f)),
	strength(1.f),
	shininess(32),
	falloff(LINEAR)
{
}
