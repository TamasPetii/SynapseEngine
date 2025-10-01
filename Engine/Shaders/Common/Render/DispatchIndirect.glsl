#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct DispatchIndirectCommand
{
    uint num_groups_x;
    uint num_groups_y;
    uint num_groups_z;
};


layout(buffer_reference, std430) buffer DispatchIndirectCommandBuffer {
	DispatchIndirectCommand command;
};