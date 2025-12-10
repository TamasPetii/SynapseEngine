#ifndef COMMON_MODEL_BUFFER_ADDRESSES_GLSL
#define COMMON_MODEL_BUFFER_ADDRESSES_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct ModelDeviceAddresses {
	uvec2 vertexBuffer;
	uvec2 indexBuffer;
	uvec2 materialIndexBuffer;
	uvec2 nodeTransformBuffer;
}; 

layout(buffer_reference, std430) readonly buffer ModelDeviceAddressesBuffer { 
	ModelDeviceAddresses deviceAddresses[];
};

#endif