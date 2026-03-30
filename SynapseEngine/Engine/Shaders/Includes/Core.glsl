#ifndef SYN_INCLUDES_CORE_GLSL
#define SYN_INCLUDES_CORE_GLSL

#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#define PI 3.14159265359
#define INV_PI 0.31830988618379067154
#define EPSILON 1e-5
#define INVALID_INDEX 0xFFFFFFFFu

#define SATURATE(x) clamp(x, 0.0, 1.0)

#define HAS_FLAG(value, flagBit) (((value) & (1u << (flagBit))) != 0u)
#define SET_FLAG(value, flagBit)   ((value) |= (1u << (flagBit)))
#define CLEAR_FLAG(value, flagBit) ((value) &= ~(1u << (flagBit)))
#define TOGGLE_FLAG(value, flagBit) ((value) ^= (1u << (flagBit)))
#define SET_BIT_TO(value, bit, condition) ((condition) ? ((value) | (1u << (bit))) : ((value) & ~(1u << (bit))))

#define PACK_UINT16(x, y)       (((x) & 0xFFFFu) | (((y) & 0xFFFFu) << 16u))
#define UNPACK_UINT16_X(packed) ((packed) & 0xFFFFu)
#define UNPACK_UINT16_Y(packed) (((packed) >> 16u) & 0xFFFFu)

#define NDC_TO_UV(ndc) ((ndc) * 0.5 + 0.5)

layout(buffer_reference, std430) readonly buffer SparseMapBuffer { 
    uint data[]; 
};

#define GET_SPARSE_INDEX(addr, entityId) SparseMapBuffer(addr).data[entityId]
#define HAS_COMPONENT(addr, entityId) (SparseMapBuffer(addr).data[entityId] != INVALID_INDEX)

#endif