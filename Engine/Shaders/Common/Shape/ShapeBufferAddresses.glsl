#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct ShapeDeviceAddresses {
	uvec2 vertexBuffer;
	uvec2 indexBuffer;
}; 

layout(buffer_reference, std430) readonly buffer ShapeDeviceAddressesBuffer { 
	ShapeDeviceAddresses deviceAddresses[];
};