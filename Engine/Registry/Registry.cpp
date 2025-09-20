#include "Registry.h"
#include "Relationship.h"
#include "Engine/Components/RenderIndicesComponent.h"

Registry::Registry()
{
}

Entity Registry::CreateEntity()
{
	Entity entity = NULL_ENTITY;

	if (destroyedEntities.empty())
	{
		entity = counter++;
	}
	else
	{
		auto it = destroyedEntities.begin();
		entity = *it;
		destroyedEntities.erase(it);
	}

	AddComponents<ComponentBitsetMask, Relationship>(entity);
	levels[0].insert(entity);
	return entity;
}

void Registry::DestroyEntity(Entity entity)
{
	//Remove the parent-child relations

	//Remove from all the component pools
	for (auto& pool : pools.GetDenseData())
			pool->RemoveComponent(entity);

	destroyedEntities.insert(entity);
}

void Registry::SetActiveEntity(Entity entity)
{
	activeEntity = entity;
}

Entity Registry::GetActiveEntity()
{
	return activeEntity;
}

void Registry::SetParent(Entity entity, Parent parent)
{
	//Not letting parent-child relation with the same entity
	if (entity == parent)
		return;

	auto& entityRelationShip = GetComponent<Relationship>(entity);

	//Not letting the same parent to be set.. no point...
	if (parent != NULL_ENTITY && entityRelationShip.parent == parent)
		return;

	//Erase the entity from previous parents children vector.
	if (entityRelationShip.parent != NULL_ENTITY)
	{
		auto& parentRelationship = GetComponent<Relationship>(entityRelationShip.parent);
		parentRelationship.children.erase(entity);
	}

	//Set entity parent to new entity
	entityRelationShip.parent = parent;

	//Add entity to new parent children vector
	if (entityRelationShip.parent != NULL_ENTITY)
	{
		auto& parentRelationship = GetComponent<Relationship>(entityRelationShip.parent);
		parentRelationship.children.insert(entity);

		//New parent on same level, no need to update cached entity levels
		if (entityRelationShip.level == parentRelationship.level + 1)
			return;
	}

	//Update cached entity levels, and update entity level regardless of valid or invalid parent!
	//Recursively for all entities
	RefreshChildLevelDeep(entity);
}

void Registry::RefreshChildLevelDeep(Entity entity)
{
	auto& entityRelationShip = GetComponent<Relationship>(entity);
	
	levels[entityRelationShip.level].erase(entity);
	entityRelationShip.level = entityRelationShip.parent == NULL_ENTITY ? 0 : GetComponent<Relationship>(entityRelationShip.parent).level + 1;
	levels[entityRelationShip.level].insert(entity);

	//Update all childred level
	for (auto child : entityRelationShip.children)
		RefreshChildLevelDeep(child);
}

bool Registry::IsDeepConnected(Entity entity, Parent parent)
{
	if (entity == NULL_ENTITY)
		return false;

	Entity currentParent = GetComponent<Relationship>(entity).parent;

	while (currentParent != NULL_ENTITY)
	{
		if (currentParent == parent)
			return true;

		currentParent = GetComponent<Relationship>(currentParent).parent;
	}

	return false;
}