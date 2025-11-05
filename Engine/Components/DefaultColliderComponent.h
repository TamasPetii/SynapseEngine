#pragma once
#include "BaseComponents/Component.h"
#include "Engine/Physics/Collider/ColliderAABB.h"
#include "Engine/Physics/Collider/ColliderOBB.h"
#include "Engine/Physics/Collider/SphereColliderGJK.h"

struct ENGINE_API DefaultColliderComponent : public Component, public SphereColliderGJK, public ColliderAABB, public ColliderOBB
{
};

struct ENGINE_API DefaultCameraCollider : public ColliderAABB, public ColliderOBB
{
};

struct ENGINE_API DefaultColliderGPU
{
	DefaultColliderGPU(const DefaultColliderComponent& component, uint32_t objectIndex, bool isModel);

	glm::vec3 aabbOrigin;
	uint32_t objectIndex; // With render indices this index will not be needed anymore -> depth can be here
	glm::vec3 aabbExtents;
	uint32_t bitflag;
	glm::vec3 sphereOrigin;
	float sphereRadius;
};