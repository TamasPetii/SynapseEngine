#pragma once
#include "EngineApi.h"
#include <cstdint>
#include <glm/glm.hpp>

struct ENGINE_API GlobalConfig
{
	struct ENGINE_API WireframeConfig
	{
		static inline bool showColliderAABB = false;
		static inline bool showColliderOBB = false;
		static inline bool showColliderSphere = false;
		static inline bool showPointLights = false;
		static inline bool showSpotLights = false;
	};

	struct ENGINE_API BillboardConfig
	{
		static inline bool showDirectionLights = true;
		static inline bool showPointLights = true;
		static inline bool showSpotLights = true;
	};

	struct ENGINE_API FrameConfig
	{
		static inline constexpr uint32_t maxFramesInFlights = 3;
		static inline uint32_t framesInFlight = 2;
	};

	struct ENGINE_API World
	{
		static constexpr glm::vec3 up = glm::vec3(0, 1, 0);
	};	
};