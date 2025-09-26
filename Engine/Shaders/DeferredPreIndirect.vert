#version 460
#extension GL_ARB_shader_draw_parameters : enable

#include "Common/Animation/AnimationNodeTransform.glsl"
#include "Common/Animation/AnimationVertexBone.glsl"
#include "Common/Animation/AnimationBufferAddresses.glsl"

#include "Common/Model/ModelNodeTransform.glsl"
#include "Common/Model/ModelBufferAddresses.glsl"
#include "Common/Shape/ShapeBufferAddresses.glsl"

#include "Common/Render/InstanceIndex.glsl"
#include "Common/Render/RenderDefines.glsl"
#include "Common/Render/RenderIndices.glsl"

#include "Common/Camera.glsl"
#include "Common/Vertex.glsl"
#include "Common/Material.glsl"
#include "Common/Transform.glsl"
#include "Common/Index.glsl"

layout (location = 0) out vec3 vs_out_pos;
layout (location = 1) out vec3 vs_out_norm;
layout (location = 2) out vec2 vs_out_tex;
layout (location = 3) out flat uvec4 vs_out_index;
layout (location = 4) out mat3 vs_out_tbn;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 transformBuffer;
	uvec2 instanceIndexAddressBuffer;
	uvec2 modelRenderIndicesBuffer;
	uvec2 modelBufferAddresses;
	uvec2 animationTransformBufferAddresses;
	uvec2 animationVertexBoneBufferAddresses;
	uvec2 shapeRenderIndicesBuffer;
	uvec2 shapeBufferAddresses;
	uvec2 shapeMaterialIndicesBuffer;
	uvec2 materialBuffer;
	uint renderMode;
	uint cameraIndex;
} PushConstants;

void main() 
{
	uvec2 instanceIndexBuffer = InstanceIndexAddressBuffer(PushConstants.instanceIndexAddressBuffer).deviceAddresses[gl_DrawID];

    uint index = InstanceIndexBuffer(instanceIndexBuffer).indices[gl_InstanceIndex];

	uint entityIndex = INVALID_RENDER_INDEX;
	uint transformIndex = INVALID_RENDER_INDEX;
	uint meshIndex = INVALID_RENDER_INDEX;
	uint materialIndex = INVALID_RENDER_INDEX;
	uint vertexIndex = INVALID_RENDER_INDEX;
	uint bitflag = 0;

	Vertex v;

	if(PushConstants.renderMode == MODEL_INSTANCED)
	{
		ModelRenderIndices renderIndices = ModelRenderIndicesBuffer(PushConstants.modelRenderIndicesBuffer).indices[index];
		entityIndex = renderIndices.entityIndex;
		transformIndex = renderIndices.transformIndex;
		meshIndex = renderIndices.modelIndex;
		bitflag = renderIndices.bitflag;

		vertexIndex = IndexBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].indexBuffer).indices[gl_VertexIndex];	
		v = VertexBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].vertexBuffer).vertices[vertexIndex];
		
		materialIndex = MaterialIndexBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].materialIndexBuffer).materialIndices[v.matIndex];
	}
	else if(PushConstants.renderMode == SHAPE_INSTANCED)
	{
		ShapeRenderIndices renderIndices = ShapeRenderIndicesBuffer(PushConstants.shapeRenderIndicesBuffer).indices[index];
		entityIndex = renderIndices.entityIndex;
		transformIndex = renderIndices.transformIndex;
		meshIndex = renderIndices.shapeIndex;
		bitflag = renderIndices.bitflag;

		vertexIndex = IndexBuffer(ShapeDeviceAddressesBuffer(PushConstants.shapeBufferAddresses).deviceAddresses[meshIndex].indexBuffer).indices[gl_VertexIndex];	
		v = VertexBuffer(ShapeDeviceAddressesBuffer(PushConstants.shapeBufferAddresses).deviceAddresses[meshIndex].vertexBuffer).vertices[vertexIndex];
		
		materialIndex = MaterialIndexBuffer(PushConstants.shapeMaterialIndicesBuffer).materialIndices[renderIndices.materialIndex];
	}

	vec4 position = vec4(v.position, 1.0);
	vec4 normal = vec4(v.normal, 0.0);
	vec4 tangent = vec4(v.tangent, 0.0);
	vec4 bitangent = vec4(v.bitangent, 0.0);

	if(PushConstants.renderMode == MODEL_INSTANCED)
	{	
		ModelRenderIndices renderIndices = ModelRenderIndicesBuffer(PushConstants.modelRenderIndicesBuffer).indices[index];

		//Has valid animation, render dynamic model
		if(renderIndices.animationIndex != INVALID_RENDER_INDEX && renderIndices.animationTransformIndex != INVALID_RENDER_INDEX)
		{
			AnimationNodeTransformBuffer animationNodeTransformBuffer = AnimationNodeTransformBuffer(AnimationNodeTransformDeviceAddressesBuffer(PushConstants.animationTransformBufferAddresses).deviceAddresses[renderIndices.animationTransformIndex]);	
			AnimationVertexBone vertexBone = AnimationVertexBoneBuffer(AnimationVertexBoneDeviceAddressesBuffer(PushConstants.animationVertexBoneBufferAddresses).deviceAddresses[renderIndices.animationIndex]).vertexBones[vertexIndex];
			
			bool hasBone = vertexBone.indices.x != INVALID_VERTEX_BONE_INDEX || 
						   vertexBone.indices.y != INVALID_VERTEX_BONE_INDEX || 
						   vertexBone.indices.z != INVALID_VERTEX_BONE_INDEX ||
						   vertexBone.indices.w != INVALID_VERTEX_BONE_INDEX;

			if(hasBone)
			{
				vec4 totalPosition = vec4(0);
				vec4 totalNormal = vec4(0);
				vec4 totalTanget = vec4(0);
				vec4 totalBitangent = vec4(0);

				for(int i = 0; i < 4; i++)
				{
					if(vertexBone.indices[i] == INVALID_VERTEX_BONE_INDEX)
						continue;

					totalPosition += (animationNodeTransformBuffer.nodeTransforms[vertexBone.indices[i]].transform * position) * vertexBone.weights[i];
					totalNormal += (animationNodeTransformBuffer.nodeTransforms[vertexBone.indices[i]].transformIT * normal) * vertexBone.weights[i];
					totalTanget += (animationNodeTransformBuffer.nodeTransforms[vertexBone.indices[i]].transformIT * tangent) * vertexBone.weights[i];
					totalBitangent += (animationNodeTransformBuffer.nodeTransforms[vertexBone.indices[i]].transformIT * bitangent) * vertexBone.weights[i];
				}
					
				position = totalPosition;
				normal = totalNormal;
				tangent = totalTanget;
				bitangent = totalBitangent;
			}
			else
			{
				position = animationNodeTransformBuffer.nodeTransforms[v.nodeIndex].transform * position;
				normal = animationNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * normal;
				tangent = animationNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * tangent;
				bitangent = animationNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * bitangent;
			}
		}
		//No animation render static model
		else
		{
			ModelNodeTransformBuffer modelNodeTransformBuffer = ModelNodeTransformBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[renderIndices.modelIndex].nodeTransformBuffer);
			position = modelNodeTransformBuffer.nodeTransforms[v.nodeIndex].transform * position;
			normal = modelNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * normal;
			tangent = modelNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * tangent;
			bitangent = modelNodeTransformBuffer.nodeTransforms[v.nodeIndex].transformIT * bitangent;
		}
	}

	vec4 worldPosition = TransformBuffer(PushConstants.transformBuffer).transforms[transformIndex].transform * position;
	gl_Position = CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].viewProj * worldPosition;

	vec3 finalNormal = normalize((TransformBuffer(PushConstants.transformBuffer).transforms[transformIndex].transformIT * normal).xyz);
	vec3 finalTangent = normalize((TransformBuffer(PushConstants.transformBuffer).transforms[transformIndex].transformIT * tangent).xyz);
	finalTangent = normalize(finalTangent - dot(finalTangent, finalNormal) * finalNormal);
	vec3 finalBitangent = normalize(cross(finalNormal, finalTangent));
	//finalBitangent = normalize((TransformBuffer(PushConstants.transformBuffer).transforms[transformIndex].transformIT * bitangent).xyz);

	vs_out_pos = worldPosition.xyz;
	vs_out_norm = finalNormal;
	vs_out_tex = vec2(v.uv_x, 1.0 - v.uv_y);
	vs_out_index.x = entityIndex; // Need to store entity index into texture -> Object picking
	vs_out_index.y = meshIndex; //Need model array index to get material buffer
	vs_out_index.z = materialIndex; //Vertex Material Index | Material buffer index for shapes
	vs_out_index.w = bitflag; //Stores invert normal, and shadow flags
	vs_out_tbn = mat3(finalTangent, finalBitangent, finalNormal);
}