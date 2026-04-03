#include "Registry.h"
#include <algorithm>

namespace Syn
{
	Registry::Registry()
	{
	}

	Registry::~Registry()
	{
		Clear();
	}

	EntityID Registry::CreateEntity()
	{
		EntityID entity;

		if (_freeEntities.empty())
		{
			entity = _entityCounter++;

			std::for_each(_pools.GetDenseEntities().begin(), _pools.GetDenseEntities().end(),
				[&](EntityID poolEntity) {
					IPool* pool = _pools.Get(poolEntity);
					pool->EnsureEntityMapping(entity);
				});
		}
		else
		{
			entity = _freeEntities.back();
			_freeEntities.pop_back();
		}

		_activeEntities.Add(entity);
		return entity;
	}

	void Registry::DestroyEntity(EntityID entity)
	{
		if (!_activeEntities.Has(entity))
			return;

		std::for_each(_pools.GetDenseEntities().begin(), _pools.GetDenseEntities().end(), 
			[&](EntityID poolEntity) {
				IPool* pool = _pools.Get(poolEntity);
				pool->RemoveIfHas(entity);
			});

		_activeEntities.Remove(entity);
		_freeEntities.push_back(entity);
	}

	bool Registry::IsValid(EntityID entity) const
	{
		return _activeEntities.Has(entity);
	}

	void Registry::Clear()
	{
		std::for_each(_pools.GetDenseEntities().begin(), _pools.GetDenseEntities().end(),
			[&](EntityID poolEntity) {
				IPool* pool = _pools.Get(poolEntity);
				pool->Clear();
				delete pool;
			});

		_pools.Clear();
		_activeEntities.Clear();
		_freeEntities.clear();
		_entityCounter = 0;
	}
}