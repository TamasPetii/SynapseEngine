#ifndef COMMON_SHAPE_BUFFER_ADDRESSES_GLSL
#define COMMON_SHAPE_BUFFER_ADDRESSES_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct ShapeDeviceAddresses {
	uvec2 vertexBuffer;
	uvec2 indexBuffer;
}; 

layout(buffer_reference, std430) readonly buffer ShapeDeviceAddressesBuffer { 
	ShapeDeviceAddresses deviceAddresses[];
};

#endif