#pragma once
#pragma warning(disable: 4251)

#ifdef SYN_BUILD_DLL
#define SYN_API __declspec(dllexport)
#else
#define SYN_API __declspec(dllimport)
#endif