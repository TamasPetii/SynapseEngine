#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct LightCommonData {
	uvec3 aabbMin;
	uint count;
	uvec3 aabbMax;
	uint shadowCount;
};

layout(buffer_reference, std430) buffer LightCommonDataBuffer { 
	LightCommonData commonData;
};