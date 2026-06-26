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
			#if defined(_MSC_VER)
            return __FUNCSIG__;
			#elif defined(__clang__) || defined(__GNUC__)
						return __PRETTY_FUNCTION__;
			#else
						#error "Unsupported compiler for TypeInfo generation."
			#endif
		}
	};

	template<typename T>
	const TypeID TypeInfo<T>::ID = TypeManager::Get().GetOrRegisterID(TypeInfo<T>::GetName());
}