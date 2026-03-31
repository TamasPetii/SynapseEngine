#ifndef SYN_SHARED_GPU_TYPES_H
#define SYN_SHARED_GPU_TYPES_H

#ifdef __cplusplus
    #include <cstdint>
    #include <glm/glm.hpp>

    using uint  = uint32_t;
    using uint64_t  = uint64_t;
    using vec2  = glm::vec2;
    using vec3  = glm::vec3;
    using vec4  = glm::vec4;
    using ivec2 = glm::ivec2;
    using ivec3 = glm::ivec3;
    using ivec4 = glm::ivec4;
    using uvec2 = glm::uvec2;
    using uvec3 = glm::uvec3;
    using uvec4 = glm::uvec4;
    using mat3  = glm::mat3;
    using mat4  = glm::mat4;

    #define ALIGN(x) alignas(x)
#else
    #define ALIGN(x) 
#endif

#endif