#pragma once
#include "SparseSet.h"
#include <stdexcept>

template<typename T>
class DataPool : public virtual SparseSet
{
public:
	virtual ~DataPool<T>() = default;
	virtual void Remove(uint32_t index) override;
	virtual void Add(uint32_t index) override;
	virtual void Add(uint32_t index, const T& data);
	T& GetData(uint32_t index);
	const auto& GetDenseData() { return denseDatas; }
protected:
	void RemoveData(const RemoveContext& context);
	void AddData(const T& data);
protected:
	std::vector<T> denseDatas;
};

template<typename T>
inline void DataPool<T>::Remove(uint32_t index)
{
	auto context = GetRemoveContext(index);
	if (context.has_value())
	{
		DataPool<T>::RemoveData(context.value());
		SparseSet::RemoveIndex(context.value());
	}
}

template<typename T>
inline void DataPool<T>::Add(uint32_t index)
{
	static_assert(std::is_default_constructible<T>::value, "DataPool<T>::AddData T must be default constructible.");
	Add(index, T{});
}

template<typename T>
inline void DataPool<T>::Add(uint32_t index, const T& data)
{
	auto context = GetAddContext(index);
	SparseSet::RegisterIndex(index);

	if (context.has_value())
	{
		SparseSet::AddIndex(context.value());
		DataPool<T>::AddData(data);
	}
}

template<typename T>
inline T& DataPool<T>::GetData(uint32_t index)
{
	[[unlikely]]
	if (!ContainsIndex(index))
		throw std::runtime_error("Invalid index for accesing bitset component!");

	return denseDatas[sparseIndices[GetPageIndex(index)][GetPageOffset(index)]];
}

template<typename T>
inline void DataPool<T>::RemoveData(const RemoveContext& context)
{
	std::swap(denseDatas[context.deleteDenseIndex], denseDatas[context.swapDenseIndex]);
	denseDatas.pop_back();
}

template<typename T>
inline void DataPool<T>::AddData(const T& data)
{
	denseDatas.emplace_back(data);
}
