#include "BitsetPool.h"
#include <stdexcept>

void BitsetPool::Remove(uint32_t index)
{
	auto context = GetRemoveContext(index);
	if (context.has_value())
	{
		BitsetPool::RemoveBitset(context.value());
		SparseSet::RemoveIndex(context.value());
	}
}

void BitsetPool::Add(uint32_t index)
{
	auto context = GetAddContext(index);
	SparseSet::RegisterIndex(index);

	if (context.has_value())
	{
		SparseSet::AddIndex(context.value());
		BitsetPool::AddBitset(index);
	}
}

BitsetFlag& BitsetPool::GetBitset(uint32_t index)
{
	[[unlikely]]
	if (!ContainsIndex(index))
		throw std::runtime_error("Invalid index for accesing bitset component!");

	return denseBitsets[sparseIndices[GetPageIndex(index)][GetPageOffset(index)]];
}

void BitsetPool::AddBitset(uint32_t index)
{
	denseBitsets.push_back(BitsetFlag{});
	SetBit<REGENERATE_BIT, UPDATE_BIT, INDEX_CHANGED_BIT>(index);
}

void BitsetPool::RemoveBitset(const RemoveContext& context)
{
	SetBit<REGENERATE_BIT, UPDATE_BIT, INDEX_CHANGED_BIT>(denseIndices[context.swapDenseIndex]);
	std::swap(denseBitsets[context.deleteDenseIndex], denseBitsets[context.swapDenseIndex]);
	denseBitsets.pop_back();
}
