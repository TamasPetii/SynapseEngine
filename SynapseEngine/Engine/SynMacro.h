#pragma once
#include <print>
#include <cstdlib>

#ifndef SYN_DIST
#define ENABLE_CHECKS
#endif

namespace Syn {

    #ifdef SYN_DIST
        constexpr bool IsDist = true;
    #else
        constexpr bool IsDist = false;
    #endif

    constexpr bool EnableLogging = !IsDist;
    constexpr bool EnableValidation = !IsDist;

    inline void HandleAssert(const char* msg, const char* file, int line) {
        std::println(stderr, "CRITICAL ERROR: {}\n\tAt: {}:{}", msg, file, line);
        std::abort();
    }
}


#ifdef ENABLE_CHECKS

#define SYN_ASSERT(cond, msg) \
        if (!(cond)) Syn::HandleAssert(msg, __FILE__, __LINE__)

#define SYN_CHECK(cond) \
        if (!(cond)) return

#define SYN_CHECK_RETURN(cond, val) \
        if (!(cond)) return val

#else

#define SYN_ASSERT(cond, msg)
#define SYN_CHECK(cond)
#define SYN_CHECK_RETURN(cond, val)

#endif

#if defined(_MSC_VER)
#define SYN_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define SYN_INLINE __attribute__((always_inline)) inline
#else
#define SYN_INLINE inline
#endif