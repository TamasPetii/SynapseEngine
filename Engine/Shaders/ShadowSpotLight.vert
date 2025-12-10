#version 460
#extension GL_ARB_shader_draw_parameters : require

#include "Common/Animation/AnimationNodeTransform.glsl"
#include "Common/Animation/AnimationVertexBone.glsl"
#include "Common/Animation/AnimationBufferAddresses.glsl"

#include "Common/Model/ModelNodeTransform.glsl"
#include "Common/Model/ModelBufferAddresses.glsl"
#include "Common/Shape/ShapeBufferAddresses.glsl"

#include "Common/Render/InstanceIndex.glsl"
#include "Common/Render/RenderDefines.glsl"
#include "Common/Render/RenderIndices.glsl"

#include "Common/Vertex.glsl"
#include "Common/Transform.glsl"
#include "Common/Index.glsl"

#include "Common/Light/SpotLight.glsl"

layout( push_constant ) uniform constants
{   
    uvec2 transformBuffer;
    uvec2 instanceIndexAddressBuffer;
    uvec2 modelRenderIndicesBuffer;
    uvec2 modelBufferAddresses;
    uvec2 animationTransformBufferAddresses;
    uvec2 animationVertexBoneBufferAddresses;
    uvec2 shapeRenderIndicesBuffer;
    uvec2 shapeBufferAddresses;
    uvec2 spotLightBufferAddress; 
    uvec2 spotLightShadowBufferAddress;
    uint renderMode;
    uint spotLightIndex;
} PushConstants;

void main() 
{
    uvec2 instanceIndexBuffer = InstanceIndexAddressBuffer(PushConstants.instanceIndexAddressBuffer).deviceAddresses[gl_DrawIDARB];
    uint index = InstanceIndexBuffer(instanceIndexBuffer).indices[gl_InstanceIndex];

    uint transformIndex = INVALID_RENDER_INDEX;
    uint meshIndex = INVALID_RENDER_INDEX;
    uint vertexIndex = INVALID_RENDER_INDEX;

    Vertex v;

    if(PushConstants.renderMode == MODEL_INSTANCED)
    {
        ModelRenderIndices renderIndices = ModelRenderIndicesBuffer(PushConstants.modelRenderIndicesBuffer).indices[index];
        transformIndex = renderIndices.transformIndex;
        meshIndex = renderIndices.modelIndex;

        vertexIndex = IndexBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].indexBuffer).indices[gl_VertexIndex];     
        v = VertexBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].vertexBuffer).vertices[vertexIndex];     
    }
    else if(PushConstants.renderMode == SHAPE_INSTANCED)
    {
        ShapeRenderIndices renderIndices = ShapeRenderIndicesBuffer(PushConstants.shapeRenderIndicesBuffer).indices[index];
        transformIndex = renderIndices.transformIndex;
        meshIndex = renderIndices.shapeIndex;

        vertexIndex = IndexBuffer(ShapeDeviceAddressesBuffer(PushConstants.shapeBufferAddresses).deviceAddresses[meshIndex].indexBuffer).indices[gl_VertexIndex];     
        v = VertexBuffer(ShapeDeviceAddressesBuffer(PushConstants.shapeBufferAddresses).deviceAddresses[meshIndex].vertexBuffer).vertices[vertexIndex];     
    }

    vec4 position = vec4(v.position, 1.0);

    if(PushConstants.renderMode == MODEL_INSTANCED)
    {     
        ModelRenderIndices renderIndices = ModelRenderIndicesBuffer(PushConstants.modelRenderIndicesBuffer).indices[index];

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
                for(int i = 0; i < 4; i++)
                {
                    if(vertexBone.indices[i] == INVALID_VERTEX_BONE_INDEX) continue;
                    totalPosition += (animationNodeTransformBuffer.nodeTransforms[vertexBone.indices[i]].transform * position) * vertexBone.weights[i];
                }
                position = totalPosition;
            }
            else
            {
                position = animationNodeTransformBuffer.nodeTransforms[v.nodeIndex].transform * position;
            }
        }
        else
        {
            ModelNodeTransformBuffer modelNodeTransformBuffer = ModelNodeTransformBuffer(ModelDeviceAddressesBuffer(PushConstants.modelBufferAddresses).deviceAddresses[meshIndex].nodeTransformBuffer);
            position = modelNodeTransformBuffer.nodeTransforms[v.nodeIndex].transform * position;
        }
    }

    vec4 worldPosition = TransformBuffer(PushConstants.transformBuffer).transforms[transformIndex].transform * position;
    uint shadowIndex = SpotLightBuffer(PushConstants.spotLightBufferAddress).lights[PushConstants.spotLightIndex].shadowIndex;

    gl_Position = SpotLightShadowBuffer(PushConstants.spotLightShadowBufferAddress).lightShadows[shadowIndex].viewProj * worldPosition;
}