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
		return std::make_pair(materials.at(name), true);

	auto material = std::make_shared<Material>();
	material->SetBufferArrayIndex(GetAvailableIndex());
	materials.insert(std::make_pair(name, material));

	return std::make_pair(materials.at(name), false);
}

void MaterialManager::Update()
{
	std::lock_guard<std::mutex> lock(asyncMutex);

	for (auto& [name, material] : materials)
	{
		if (material == nullptr)
			continue;

		if (material->IsBitSet<UPDATE_BIT>())
		{
			static_cast<MaterialGPU*>(deviceAddresses->GetHandler())[material->GetBufferArrayIndex()] = MaterialGPU(material);
			material->ResetBit<UPDATE_BIT>();

			std::cout << "Material Updated" << std::endl;

			// Only if the index changed -> So shape/material can track it -> But no changes currently with this logics
			// material->SetBit<CHANGED_BIT>(); 
		}
	}
}