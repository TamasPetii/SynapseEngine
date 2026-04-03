#pragma once
#include "Engine/SynApi.h"
#include <cstdlib>

namespace Syn {

#ifdef SYN_DIST
    constexpr bool IsDist = true;
#else
    constexpr bool IsDist = false;
#endif

    constexpr bool EnableLogging = !IsDist;
    constexpr bool EnableValidation = !IsDist;

    SYN_API void HandleAssert(const char* msg, const char* file, int line);
    SYN_API void HandleVkAssert(int result, const char* expr, const char* file, int line);
    SYN_API void HandleVkAssertMsg(int result, const char* expr, const char* msg, const char* file, int line);
}

#define SYN_ASSERT(cond, msg) \
            if (!(cond)) Syn::HandleAssert(msg, __FILE__, __LINE__)

#define SYN_CHECK(cond) \
            if (!(cond)) return

#define SYN_CHECK_RETURN(cond, val) \
            if (!(cond)) return val

#define SYN_VK_ASSERT(expr) \
            Syn::HandleVkAssert((int)(expr), #expr, __FILE__, __LINE__)

#define SYN_VK_ASSERT_MSG(expr, msg) \
            Syn::HandleVkAssertMsg((int)(expr), #expr, msg, __FILE__, __LINE__)

#if defined(_MSC_VER)
#define SYN_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define SYN_INLINE __attribute__((always_inline)) inline
#else
#define SYN_INLINE inline
#endif