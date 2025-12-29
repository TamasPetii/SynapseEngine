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

    inline void HandleVkAssert(int result, const char* expr, const char* file, int line) {
        if (result != 0) {
            std::println(stderr, "VULKAN ERROR: {} (Code: {})\n\tAt: {}:{}", expr, result, file, line);
            std::abort();
        }
    }

    inline void HandleVkAssertMsg(int result, const char* expr, const char* msg, const char* file, int line) {
        if (result != 0) {
            std::println(stderr, "VULKAN ERROR: {}\n\tExpression: {} (Code: {})\n\tAt: {}:{}", msg, expr, result, file, line);
            std::abort();
        }
    }
}

#ifdef ENABLE_CHECKS

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

#else

#define SYN_ASSERT(cond, msg)
#define SYN_CHECK(cond)
#define SYN_CHECK_RETURN(cond, val)
#define SYN_VK_ASSERT(expr) (expr)
#define SYN_VK_ASSERT_MSG(expr, msg) (expr)

#endif

#if defined(_MSC_VER)
#define SYN_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define SYN_INLINE __attribute__((always_inline)) inline
#else
#define SYN_INLINE inline
#endif