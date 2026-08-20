// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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

				if (auto it = _destroyCallbacks.find(poolEntity); it != _destroyCallbacks.end())
				{
					for (auto& wrapper : it->second)
					{
						wrapper->Invoke(entity, pool);
					}
				}

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