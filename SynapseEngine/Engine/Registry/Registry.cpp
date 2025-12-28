#include "Registry.h"

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

		const size_t poolCount = _pools.Size();
		for (size_t i = 0; i < poolCount; ++i)
		{
			_pools._storage.Get(static_cast<DenseIndex>(i))->RemoveIfHas(entity);
		}

		_activeEntities.Remove(entity);
		_freeEntities.push_back(entity);
	}

	bool Registry::IsValid(EntityID entity) const
	{
		return _activeEntities.Has(entity);
	}

	void Registry::Clear()
	{
		const size_t poolCount = _pools.Size();
		for (size_t i = 0; i < poolCount; ++i)
		{
			IPool* pool = _pools._storage.Get(static_cast<DenseIndex>(i));
			pool->Clear();
			delete pool;
		}
		_pools.Clear();

		_activeEntities.Clear();
		_freeEntities.clear();
		_entityCounter = 0;
	}
}