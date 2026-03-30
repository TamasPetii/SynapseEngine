#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "BaseComponent/Light.h"
#include <glm/glm.hpp>
#include <array>

namespace Syn
{
	struct SYN_API DirectionLightComponent : public Component, public Light
	{
		DirectionLightComponent();

        glm::vec3 direction;
		float shadowFarPlane;
		glm::vec4 cascadeSplits;
		std::array<glm::mat4, 4> cascadeViewProjs; 
		std::array<glm::vec4, 4> cascadeAtlasRects;
	};

    struct SYN_API DirectionLightGPU
    {
        DirectionLightGPU(const DirectionLightComponent& component);

        glm::vec3 direction;
        float strength;
        glm::vec3 color;
        uint32_t flags;

        glm::vec4 cascadeSplits;
        glm::mat4 cascadeViewProjs[4];
        glm::vec4 cascadeAtlasRects[4];
    };
}