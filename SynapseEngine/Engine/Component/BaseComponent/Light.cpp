#include "Light.h"

namespace Syn
{
    Light::Light() :
        color(glm::vec3(1.0f)),
        strength(1.0f),
        useShadow(false)
    {}
}