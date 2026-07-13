#ifndef SYN_INCLUDES_UTILS_ENVIRONMENT_MATH_GLSL
#define SYN_INCLUDES_UTILS_ENVIRONMENT_MATH_GLSL

vec2 SampleEquirectangular(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.15915494309, 0.31830988618);
    uv += 0.5;
    return uv;
}

vec2 SampleOctahedral(vec3 v) {
    v /= (abs(v.x) + abs(v.y) + abs(v.z));
    vec2 uv = v.z >= 0.0 ? v.xy : (1.0 - abs(v.yx)) * sign(v.xy);
    return uv * 0.5 + 0.5;
}

#endif