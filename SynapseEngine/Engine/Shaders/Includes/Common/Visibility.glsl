#ifndef SYN_INCLUDES_COMMON_VISIBILITY_GLSL
#define SYN_INCLUDES_COMMON_VISIBILITY_GLSL

// VISIBILITY BUFFER LAYOUT (R32G32_UINT)
// Word 0 (R): [Bit 31: Pipeline Flag | Bits 0-30: Entity ID]
// Word 1 (G): [Payload (Mesh, LOD, Meshlet/Primitive)]

#define VIS_PIPELINE_BIT          31u
#define VIS_PIPELINE_MESH_SHADER  1u
#define VIS_PIPELINE_TRADITIONAL  0u

#define PACK_VISIBILITY_ENTITY(entityId, pipelineFlag) ((entityId) | ((pipelineFlag) << VIS_PIPELINE_BIT))
#define UNPACK_VISIBILITY_ENTITY(word0)                ((word0) & ~(1u << VIS_PIPELINE_BIT))
#define UNPACK_VISIBILITY_PIPELINE(word0)              ((word0) >> VIS_PIPELINE_BIT)

#define VIS_BIT_COUNT_PRIMITIVE_DATA  20
#define VIS_BIT_COUNT_LOD             2
#define VIS_BIT_COUNT_MESH            10

#define VIS_SHIFT_PRIMITIVE_DATA      0
#define VIS_SHIFT_LOD                 (VIS_SHIFT_PRIMITIVE_DATA + VIS_BIT_COUNT_PRIMITIVE_DATA)
#define VIS_SHIFT_MESH                (VIS_SHIFT_LOD + VIS_BIT_COUNT_LOD)

#define VIS_MASK_PRIMITIVE_DATA       0xFFFFFu
#define VIS_MASK_LOD                  0x03u
#define VIS_MASK_MESH                 0x3FFu

#define VIS_BIT_COUNT_MS_TRIANGLE     7
#define VIS_BIT_COUNT_MS_MESHLET      13
#define VIS_SHIFT_MS_TRIANGLE         0
#define VIS_SHIFT_MS_MESHLET          7
#define VIS_MASK_MS_TRIANGLE          0x7Fu
#define VIS_MASK_MS_MESHLET           0x1FFFu

#define PACK_PARTIAL_TRADITIONAL(lod, mesh) \
    ( (((lod)  & VIS_MASK_LOD)  << VIS_SHIFT_LOD) | \
      (((mesh) & VIS_MASK_MESH) << VIS_SHIFT_MESH) )

#define PACK_PARTIAL_MESH_SHADER(meshlet, lod, mesh) \
    ( (((meshlet) & VIS_MASK_MS_MESHLET) << VIS_SHIFT_MS_MESHLET) | \
      (((lod)     & VIS_MASK_LOD)        << VIS_SHIFT_LOD)        | \
      (((mesh)    & VIS_MASK_MESH)       << VIS_SHIFT_MESH) )

#define FINALIZE_VIS_TRADITIONAL(partial, primID) \
    ( (partial) | (((primID) & VIS_MASK_PRIMITIVE_DATA) << VIS_SHIFT_PRIMITIVE_DATA) )

#define FINALIZE_VIS_MS(partial, triIdx) \
    ( (partial) | (((triIdx) & VIS_MASK_MS_TRIANGLE) << VIS_SHIFT_MS_TRIANGLE) )

#define UNPACK_VISIBILITY_PRIMITIVE_ID(payload) (((payload) >> VIS_SHIFT_PRIMITIVE_DATA) & VIS_MASK_PRIMITIVE_DATA)
#define UNPACK_VISIBILITY_LOD(payload)          (((payload) >> VIS_SHIFT_LOD)            & VIS_MASK_LOD)
#define UNPACK_VISIBILITY_MESH(payload)         (((payload) >> VIS_SHIFT_MESH)           & VIS_MASK_MESH)
#define UNPACK_VISIBILITY_MS_TRIANGLE(payload)  (((payload) >> VIS_SHIFT_MS_TRIANGLE)    & VIS_MASK_MS_TRIANGLE)
#define UNPACK_VISIBILITY_MS_MESHLET(payload)   (((payload) >> VIS_SHIFT_MS_MESHLET)     & VIS_MASK_MS_MESHLET)

#endif