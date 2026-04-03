#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "Engine/Collision/Colliders/AabbCollider.h"
#include "Engine/Collision/Colliders/SphereCollider.h"
#include "Engine/Collision/Colliders/ConeCollider.h"

namespace Syn
{
	struct SYN_API CookedMeshCollisionData
	{
		AabbCollider aabb;
		SphereCollider sphere;
	};

	struct SYN_API CookedMeshletCollisionData
	{
		AabbCollider aabb;
		SphereCollider sphere;
		ConeCollider cone;
	};
}


