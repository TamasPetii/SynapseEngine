#pragma once
#include "DataPool.h"
#include "BitsetPool.h"
#include "Engine/Registry/Entity.h"

class ENGINE_API IComponentPool
{
public:
	virtual ~IComponentPool() = default;
	virtual bool HasComponent(Entity entity) = 0;
	virtual void RemoveComponent(Entity entity) = 0;
};

template<typename T>
class ComponentPool : public DataPool<T>, public BitsetPool, public IComponentPool
{
public:
	virtual ~ComponentPool<T>() = default;
	virtual void Remove(uint32_t index) override;
	virtual void Add(uint32_t index) override;
	virtual void Add(uint32_t index, const T& data);
	virtual bool HasComponent(Entity entity) override;
	virtual void RemoveComponent(Entity entity) override;
};

template<typename T>
inline void ComponentPool<T>::Remove(uint32_t index)
{
	auto context = GetRemoveContext(index);
	if (context.has_value())
	{
		BitsetPool::RemoveBitset(context.value());
		DataPool<T>::RemoveData(context.value());
		SparseSet::RemoveIndex(context.value());
	}
}

template<typename T>
inline void ComponentPool<T>::Add(uint32_t index)
{
	static_assert(std::is_default_constructible<T>::value, "DataPool<T>::AddData T must be default constructible.");
	Add(index, T{});
}

template<typename T>
inline void ComponentPool<T>::Add(uint32_t index, const T& data)
{
	auto context = GetAddContext(index);
	SparseSet::RegisterIndex(index);

	if (context.has_value())
	{
		SparseSet::AddIndex(context.value());
		BitsetPool::AddBitset(index);
		DataPool<T>::AddData(data);
	}
}

template<typename T>
inline void ComponentPool<T>::RemoveComponent(Entity entity)
{
	ComponentPool<T>::Remove(entity);
}

template<typename T>
inline bool ComponentPool<T>::HasComponent(Entity entity)
{
	return ComponentPool<T>::ContainsIndex(entity);
}