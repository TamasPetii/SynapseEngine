#include "Material.h"

Material::Material() :
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

MaterialGPU::MaterialGPU(std::shared_ptr<Material> material) :
	color(material->color),
	uvScale(material->uvScale),
	bloom(material->useBloom ? 1 : 0, 0),
	albedoIndex(material->albedoTexture &&  material->albedoTexture->state == LoadState::Ready ? material->albedoTexture->GetBufferArrayIndex() : UINT32_MAX),
	normalIndex(material->normalTexture && material->normalTexture->state == LoadState::Ready ? material->normalTexture->GetBufferArrayIndex() : UINT32_MAX),
	metallicIndex(material->metalnessTexture && material->metalnessTexture->state == LoadState::Ready ? material->metalnessTexture->GetBufferArrayIndex() : UINT32_MAX),
	roughnessIndex(material->roughnessTexture && material->roughnessTexture->state == LoadState::Ready ? material->roughnessTexture->GetBufferArrayIndex() : UINT32_MAX),
	metalness(material->metalness),
	roughness(material->roughness),
	padding(0)
{

}