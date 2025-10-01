#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) buffer LightCountBuffer { 
	uint lightCount;
};