#pragma once

#ifdef _MSC_VER
    #pragma warning(disable: 4251)
#endif

#if defined(_WIN32)

    #ifdef SYN_BUILD_DLL
        #define SYN_API __declspec(dllexport)
    #else
        #define SYN_API __declspec(dllimport)
    #endif

#else

    #define SYN_API __attribute__((visibility("default")))

#endif