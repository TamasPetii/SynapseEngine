#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Pool/PoolTypes.h"
#include "Pool/PoolWrapper.h"
#include "Type/TypeInfo.h"
#include <vector>
#include <tuple>

namespace Syn
{
	class SYN_API Registry
	{
	public:
		template<typename T>
		using DefaultPoolType = ComponentPool<T>;

		template<typename T>
		using WrapperType = PoolWrapper<DefaultPoolType<T>>;
	public:
		Registry();
		~Registry();

		EntityID CreateEntity();
		void DestroyEntity(EntityID entity);
		bool IsValid(EntityID entity) const;
		void Clear();

		template<typename T>
		void AddComponent(EntityID entity, T&& component);

		template<typename T>
		void AddComponent(EntityID entity);

		template<typename T>
		void RemoveComponent(EntityID entity);

		template<typename T>
		T& GetComponent(EntityID entity);

		template<typename T>
		const T& GetComponent(EntityID entity) const;

		template<typename T>
		T* TryGetComponent(EntityID entity);

		template<typename T>
		bool HasComponent(EntityID entity) const;

		template<typename... T>
		void AddComponents(EntityID entity, T&&... components);

		template<typename... T>
		void AddComponents(EntityID entity);

		template<typename... T>
		std::tuple<T&...> GetComponents(EntityID entity);

		template<typename... T>
		std::tuple<const T&...> GetComponents(EntityID entity) const;

		template<typename... T>
		void RemoveComponents(EntityID entity);

		template<typename... T>
		bool HasComponents(EntityID entity) const;

		template<typename T>
		DefaultPoolType<T>* GetPool();
	private:
		template<typename T>
		WrapperType<T>* EnsurePool();
	private:
		EntityID _entityCounter = 0;
		std::vector<EntityID> _freeEntities;
		SparseSet _activeEntities;
		DataPool<IPool*> _pools;
	};
}

namespace Syn
{
	template<typename T>
	SYN_INLINE Registry::WrapperType<T>* Registry::EnsurePool()
	{
		const TypeID id = TypeInfo<T>::ID;

		[[unlikely]]
		if (!_pools.Has(id))
		{
			auto* wrapper = new WrapperType<T>();
			_pools.Add(id, wrapper);
			return wrapper;
		}

		return static_cast<WrapperType<T>*>(_pools.Get(id));
	}

	template<typename T>
	SYN_INLINE Registry::DefaultPoolType<T>* Registry::GetPool()
	{
		const TypeID id = TypeInfo<T>::ID;

		if (!_pools.Has(id))
			return nullptr;

		return &static_cast<WrapperType<T>*>(_pools.Get(id))->_pool;
	}

	template<typename T>
	SYN_INLINE void Registry::AddComponent(EntityID entity, T&& component)
	{
		SYN_ASSERT(_activeEntities.Has(entity), "Entity invalid");
		EnsurePool<T>()->_pool.Add(entity, std::forward<T>(component));
	}

	template<typename T>
	SYN_INLINE void Registry::AddComponent(EntityID entity)
	{
		SYN_ASSERT(_activeEntities.Has(entity), "Entity invalid");
		EnsurePool<T>()->_pool.Add(entity);
	}

	template<typename T>
	SYN_INLINE void Registry::RemoveComponent(EntityID entity)
	{
		auto* pool = GetPool<T>();
		if (pool && pool->Has(entity))
		{
			pool->Remove(entity);
		}
	}

	template<typename T>
	SYN_INLINE T& Registry::GetComponent(EntityID entity)
	{
		auto* wrapper = static_cast<WrapperType<T>*>(_pools.Get(TypeInfo<T>::ID));
		SYN_ASSERT(wrapper, "Component pool missing");
		return wrapper->_pool.Get(entity);
	}

	template<typename T>
	SYN_INLINE const T& Registry::GetComponent(EntityID entity) const
	{
		const TypeID id = TypeInfo<T>::ID;
		SYN_ASSERT(_pools.Has(id), "Component pool missing");
		return static_cast<WrapperType<T>*>(_pools.Get(id))->_pool.Get(entity);
	}

	template<typename T>
	SYN_INLINE T* Registry::TryGetComponent(EntityID entity)
	{
		auto* pool = GetPool<T>();
		return pool ? pool->TryGet(entity) : nullptr;
	}

	template<typename T>
	SYN_INLINE bool Registry::HasComponent(EntityID entity) const
	{
		const TypeID id = TypeInfo<T>::ID;
		if (!_pools.Has(id))
			return false;

		return static_cast<WrapperType<T>*>(_pools.Get(id))->_pool.Has(entity);
	}

	template<typename... T>
	SYN_INLINE void Registry::AddComponents(EntityID entity, T&&... components)
	{
		(AddComponent<T>(entity, std::forward<T>(components)), ...);
	}

	template<typename... T>
	SYN_INLINE void Registry::AddComponents(EntityID entity)
	{
		(AddComponent<T>(entity), ...);
	}

	template<typename... T>
	SYN_INLINE std::tuple<T&...> Registry::GetComponents(EntityID entity)
	{
		return std::tie(GetComponent<T>(entity)...);
	}

	template<typename... T>
	SYN_INLINE std::tuple<const T&...> Registry::GetComponents(EntityID entity) const
	{
		return std::tie(GetComponent<T>(entity)...);
	}

	template<typename... T>
	SYN_INLINE void Registry::RemoveComponents(EntityID entity)
	{
		(RemoveComponent<T>(entity), ...);
	}

	template<typename... T>
	SYN_INLINE bool Registry::HasComponents(EntityID entity) const
	{
		return (HasComponent<T>(entity) && ...);
	}
}