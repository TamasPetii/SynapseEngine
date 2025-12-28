#pragma once
#include "TypeManager.h"

namespace Syn
{
	template<typename T>
	class TypeInfo {
	public:
		static const TypeID ID;
	private:
		static const char* GetName() {
			return __FUNCSIG__;
		}
	};

	template<typename T>
	const TypeID TypeInfo<T>::ID = TypeManager::Get().GetOrRegisterID(TypeInfo<T>::GetName());
}