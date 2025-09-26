#include "MaterialManager.h"

MaterialManager::MaterialManager(std::shared_ptr<ImageManager> imageManager)
{
	auto [material, wasLoaded] = RegisterMaterial("Default");
	material->albedoTexture = imageManager->LoadImage("../Assets/Engine/MissingTexture.png", ImageLoadMode::Sync, true);
}

std::pair<std::shared_ptr<Material>, bool> MaterialManager::RegisterMaterial(const std::string& name)
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	if (IsMaterialLoaded(name))
		return std::make_pair(materials.at(name)->object, true);

	auto material = std::make_shared<Material>();
	material->SetBufferArrayIndex(GetAvailableIndex());
	materials.insert(std::make_pair(name, std::make_shared<VersionedObject<Material>>(material)));

	return std::make_pair(material, false);
}

void MaterialManager::Update(uint32_t frameIndex)
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	//Todo: Mechanism to handle model delete, shrink buffer and reassign model's buffer array index!
	DeviceAddressedManager<MaterialGPU>::Update(frameIndex, ArrayIndexedManager::GetCurrentCount(), GlobalConfig::BufferConfig::materialBufferBaseSize);

	for (auto& [name, versionedObject] : materials)
	{
		if (versionedObject == nullptr)
			continue;

		auto material = versionedObject->object;

		if (material == nullptr)
			continue;

		if (material->IsBitSet<UPDATE_BIT>() || deviceAddressBuffers[frameIndex]->version != versionedObject->versions[frameIndex])
		{
			versionedObject->versions[frameIndex] = deviceAddressBuffers[frameIndex]->version;

			auto bufferHandler = static_cast<MaterialGPU*>(deviceAddressBuffers[frameIndex]->buffer->GetHandler());
			bufferHandler[material->GetBufferArrayIndex()] = MaterialGPU(material);

			material->ResetBit<UPDATE_BIT>();

			std::cout << std::format("Material {} updated in frame {} with version {}", name, frameIndex, versionedObject->versions[frameIndex]) << std::endl;

			// Only if the buffer array index changed -> So shape/material can track it -> But no changes currently with this logics
			// material->SetBit<CHANGED_BIT>(); 
		}
	}
}