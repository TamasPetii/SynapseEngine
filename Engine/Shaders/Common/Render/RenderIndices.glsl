#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#define INVALID_RENDER_INDEX 0xFFFFFFFFu

struct ModelRenderIndices
{
	uint entityIndex;
	uint transformIndex;
	uint modelIndex; //This is the loaded model array index! This is not the model component dense index!!
	uint animationIndex;
	uint animationTransformIndex;
	uint bitflag;
};

layout(buffer_reference, std430) readonly buffer ModelRenderIndicesBuffer {
	ModelRenderIndices indices[];
};

struct ShapeRenderIndices
{
	uint entityIndex;
	uint transformIndex;
	uint shapeIndex; //This is the loaded shape array index! This is not the shape component dense index!!
	uint materialIndex;
	uint bitflag;
};

layout(buffer_reference, std430) readonly buffer ShapeRenderIndicesBuffer {
	ShapeRenderIndices indices[];
};