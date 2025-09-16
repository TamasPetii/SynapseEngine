#include "MaterialComponent.h"

MaterialComponent::MaterialComponent() :
	useBloom(false),
	color(1, 1, 1, 1),
	uvScale(1, 1),
	albedoTexture(nullptr),
	normalTexture(nullptr),
	metalnessTexture(nullptr),
	roughnessTexture(nullptr),
	metalness(1.f),
	roughness(1.f)
{
}

MaterialComponentGPU::MaterialComponentGPU(const MaterialComponent& component) :
	color(component.color),
	uvScale(component.uvScale),
	bloom(component.useBloom ? 1 : 0, 0),
	albedoIndex(component.albedoTexture ? component.albedoTexture->GetDescriptorArrayIndex() : UINT32_MAX),
	normalIndex(component.normalTexture ? component.normalTexture->GetDescriptorArrayIndex() : UINT32_MAX),
	metallicIndex(component.metalnessTexture ? component.metalnessTexture->GetDescriptorArrayIndex() : UINT32_MAX),
	roughnessIndex(component.roughnessTexture ? component.roughnessTexture->GetDescriptorArrayIndex() : UINT32_MAX),
	metalness(component.metalness),
	roughness(component.roughness),
	padding(0)
{

}