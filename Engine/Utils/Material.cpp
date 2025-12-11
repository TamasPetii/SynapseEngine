#include "Material.h"

Material::Material() :
    useBloom(false),
    color(1, 1, 1, 1),
    uvScale(1, 1),
    emissiveColor(0, 0, 0),
    emissiveIntensity(0.0f),
    albedoTexture(nullptr),
    normalTexture(nullptr),
    metalnessTexture(nullptr),
    roughnessTexture(nullptr),
    emissiveTexture(nullptr),
	ambientOcclusionTexture(nullptr),
    metalness(1.f),
    roughness(1.f),
	aoStrength(1.f)
{
}

MaterialGPU::MaterialGPU(std::shared_ptr<Material> material) :
    color(material->color),
    emissiveColor(material->emissiveColor, material->useBloom ? material->emissiveIntensity : 0.0f),
    uvScale(material->uvScale),
    albedoIndex(material->albedoTexture&& material->albedoTexture->state == LoadState::Ready ? material->albedoTexture->GetBufferArrayIndex() : UINT32_MAX),
    normalIndex(material->normalTexture&& material->normalTexture->state == LoadState::Ready ? material->normalTexture->GetBufferArrayIndex() : UINT32_MAX),
    metallicIndex(material->metalnessTexture&& material->metalnessTexture->state == LoadState::Ready ? material->metalnessTexture->GetBufferArrayIndex() : UINT32_MAX),
    roughnessIndex(material->roughnessTexture&& material->roughnessTexture->state == LoadState::Ready ? material->roughnessTexture->GetBufferArrayIndex() : UINT32_MAX),
    emissiveIndex(material->emissiveTexture&& material->emissiveTexture->state == LoadState::Ready ? material->emissiveTexture->GetBufferArrayIndex() : UINT32_MAX),
	ambientOcclusionIndex(material->ambientOcclusionTexture&& material->ambientOcclusionTexture->state == LoadState::Ready ? material->ambientOcclusionTexture->GetBufferArrayIndex() : UINT32_MAX),
    metalness(material->metalness),
    roughness(material->roughness),
	aoStrength(material->aoStrength),
    padding(0)
{
}