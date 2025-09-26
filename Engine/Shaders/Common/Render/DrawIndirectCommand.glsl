#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct DrawIndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};


layout(buffer_reference, std430) buffer DrawIndirectCommandBuffer {
	DrawIndirectCommand commands[];
};