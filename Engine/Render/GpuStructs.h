#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#define SHAPE_INSTANCED 0
#define MODEL_INSTANCED 1
#define MULTIDRAW_INDIRECT_INSTANCED 2

struct GeometryRendererPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress transformBuffer;
	VkDeviceAddress instanceIndexBuffer;
	VkDeviceAddress modelRenderIndicesBuffer;
	VkDeviceAddress modelBufferAddresses;
	VkDeviceAddress animationTransformBufferAddresses;
	VkDeviceAddress animationVertexBoneBufferAddresses;
	VkDeviceAddress shapeRenderIndicesBuffer;
	VkDeviceAddress shapeBufferAddresses;
	VkDeviceAddress shapeMaterialIndicesBuffer;
	VkDeviceAddress materialBuffer;
	uint32_t renderMode;
	uint32_t cameraIndex;
	uint32_t objectIndex;
};

struct OcclusionCullingPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress transformBufferAddress;
	VkDeviceAddress instanceBufferAddress;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	VkDeviceAddress occlusionBufferAddress;
	glm::vec3 padding;
	uint32_t cameraIndex;
};

struct DeferredDirectionLightPushConstants
{	
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress directionLightBufferAddress;
	glm::vec3 padding;
	uint32_t cameraIndex;
};


struct DeferredPointLightPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress pointLightBufferAddress;
	VkDeviceAddress transformBufferAddress;
	VkDeviceAddress instanceBufferAddress;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	glm::vec2 viewPortSize;
	uint32_t cameraIndex;
	uint32_t padding;
};

struct DeferredSpotLightPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress spotLightBufferAddress;
	VkDeviceAddress transformBufferAddress;
	VkDeviceAddress instanceBufferAddress;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	glm::vec2 viewPortSize;
	uint32_t cameraIndex;
	uint32_t padding;
};

struct BoundingVolumeRendererPushConstants
{
	VkDeviceAddress cameraBufferAddress;
	VkDeviceAddress transformBufferAddress;
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
	glm::vec4 color;
	glm::vec3 padding;
	uint32_t cameraIndex;
};

struct BillboardPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress positionBufferAddress;
	VkDeviceAddress instanceBufferAddress;
	uint32_t cameraIndex;
	uint32_t iconIndex;
};

struct GeometryRendererIndirectPushConstants
{
	VkDeviceAddress cameraBuffer;
	VkDeviceAddress transformBuffer;
	VkDeviceAddress instanceIndexAddressBuffer;
	VkDeviceAddress modelRenderIndicesBuffer;
	VkDeviceAddress modelBufferAddresses;
	VkDeviceAddress animationTransformBufferAddresses;
	VkDeviceAddress animationVertexBoneBufferAddresses;
	VkDeviceAddress shapeRenderIndicesBuffer;
	VkDeviceAddress shapeBufferAddresses;
	VkDeviceAddress shapeMaterialIndicesBuffer;
	VkDeviceAddress materialBuffer;
	uint32_t renderMode;
	uint32_t cameraIndex;
};

struct CullingCameraFrustumPushConstants
{
	VkDeviceAddress cameraFrustumBuffer;
	VkDeviceAddress defaultColliderBuffer;

	VkDeviceAddress modelRenderIndicesBuffer;
	VkDeviceAddress modelBufferAddresses;
	VkDeviceAddress modelDrawIndirectCommandBuffer;
	VkDeviceAddress modelInstanceIndexAddressBuffer;

	VkDeviceAddress shapeRenderIndicesBuffer;
	VkDeviceAddress shapeBufferAddresses;
	VkDeviceAddress shapeDrawIndirectCommandBuffer;
	VkDeviceAddress shapeInstanceIndexAddressBuffer;

	uint32_t cameraIndex;
	uint32_t defaultColliderCount;
};

struct CullingPointLightPushConstants
{
	VkDeviceAddress cameraFrustumBuffer;
	VkDeviceAddress pointLightBuffer;
	VkDeviceAddress pointLightInstanceIndexBuffer;
	VkDeviceAddress pointLightDrawIndirectCommandBuffer;
	VkDeviceAddress pointLightShadowInstanceIndexBuffer;
	VkDeviceAddress pointLightCommonDataBuffer;
	VkDeviceAddress pointLightShadowDispatchIndirectBuffer;
	uint32_t cameraIndex;
	uint32_t pointLightCount;
};

struct CullingSpotLightPushConstants
{
	VkDeviceAddress cameraFrustumBuffer;
	VkDeviceAddress spotLightBuffer;
	VkDeviceAddress spotLightInstanceIndexBuffer;
	VkDeviceAddress spotLightDrawIndirectCommandBuffer;
	VkDeviceAddress spotLightShadowInstanceIndexBuffer;
	VkDeviceAddress spotLightCommonDataBuffer;
	VkDeviceAddress spotLightShadowDispatchIndirectBuffer;
	uint32_t cameraIndex;
	uint32_t spotLightCount;
};

struct CullingLightShadowAabbPushConstants
{
	VkDeviceAddress defaultColliderBuffer;
	VkDeviceAddress defaultColliderCountBuffer;
	VkDeviceAddress defaultColliderInstanceIndexBuffer;
	VkDeviceAddress pointLightCommonDataBuffer;
	VkDeviceAddress pointLightShadowDispatchIndirectBuffer;
	uint32_t defaultColliderCount;
};