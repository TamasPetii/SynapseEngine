#ifndef SYN_INCLUDES_UTILS_WBOIT_MATH_GLSL
#define SYN_INCLUDES_UTILS_WBOIT_MATH_GLSL

// McGuire Weighted Blended Order-Independent Transparency (WBOIT) weight function.
// Calculates the accumulation weight based on fragment depth (z) and opacity (alpha).
float calculateWboitWeight(float z, float alpha) {
    return clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

#endif