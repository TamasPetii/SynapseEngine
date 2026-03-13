#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"
#include <filesystem>
#include <glm/glm.hpp>

#include "Engine/Mesh/Data/Cooked/CookedMesh.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"

namespace Syn 
{
	class MeshUtils
	{
	public:
		static void LogRawModel(const std::filesystem::path& path, const RawModel& model);

		SYN_INLINE static GpuMeshCollider TransformCollider(const GpuMeshCollider& local, const glm::mat4& transform)
		{
            // 1. Forgatás és skálázás kinyerése
            glm::mat3 rotMatrix = glm::mat3(transform);
            glm::mat3 absTransform(
                glm::abs(rotMatrix[0]),
                glm::abs(rotMatrix[1]),
                glm::abs(rotMatrix[2])
            );

            // Max skálázás kiszámítása a gömb sugárhoz
            float scaleX = glm::length(rotMatrix[0]);
            float scaleY = glm::length(rotMatrix[1]);
            float scaleZ = glm::length(rotMatrix[2]);
            float maxScale = std::max({ scaleX, scaleY, scaleZ });

            // 2. Gömb transzformálása
            glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(local.center, 1.0f));
            float worldRadius = local.radius * maxScale;

            // 3. AABB transzformálása
            glm::vec3 localAabbCenter = (local.aabbMax + local.aabbMin) * 0.5f;
            glm::vec3 localAabbExtents = (local.aabbMax - local.aabbMin) * 0.5f;

            glm::vec3 worldAabbCenter = glm::vec3(transform * glm::vec4(localAabbCenter, 1.0f));
            glm::vec3 worldAabbExtents = absTransform * localAabbExtents;

            GpuMeshCollider world;
            world.center = worldCenter;
            world.radius = worldRadius;
            world.aabbMin = worldAabbCenter - worldAabbExtents;
            world.aabbMax = worldAabbCenter + worldAabbExtents;

            return world;
		}
	};
}

