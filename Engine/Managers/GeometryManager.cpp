#include "GeometryManager.h"

GeometryManager::GeometryManager()
{
	Initialize();
}

GeometryManager::~GeometryManager()
{
	Cleanup();
}

void GeometryManager::Initialize()
{
	shapes["Sphere"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Sphere>());
	shapes["Sphere"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Quad"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Quad>());
	shapes["Quad"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Capsule"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Capsule>());
	shapes["Capsule"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cube"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Cube>());
	shapes["Cube"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cylinder"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Cylinder>());
	shapes["Cylinder"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cone"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Cone>());
	shapes["Cone"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Pyramid"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Pyramid>());
	shapes["Pyramid"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Torus"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Torus>());
	shapes["Torus"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["ProxySphere"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Sphere>(16));
	shapes["ProxySphere"]->object->SetBufferArrayIndex(GetAvailableIndex());

	shapes["ProxyCone"] = std::make_shared<VersionedObject<Shape>>(std::make_shared<Cone>(16));
	shapes["ProxyCone"]->object->SetBufferArrayIndex(GetAvailableIndex());

}

void GeometryManager::Cleanup()
{
	shapes.clear();
}

std::shared_ptr<Shape> GeometryManager::GetShape(const std::string& name) const
{
	if (shapes.find(name) == shapes.end())
		return nullptr;

	if (shapes.at(name) == nullptr)
		return nullptr;

	return shapes.at(name)->object;
}

void GeometryManager::Update(uint32_t frameIndex)
{
	DeviceAddressedManager<ShapeDeviceAddresses>::Update(frameIndex, ArrayIndexedManager::GetCurrentCount(), GlobalConfig::BufferConfig::shapeAddressBufferBaseSize);
	DrawIndirectManager::Update(frameIndex, ArrayIndexedManager::GetCurrentCount(), GlobalConfig::BufferConfig::shapeAddressBufferBaseSize);

	for (auto& [path, versionedObject] : shapes)
	{
		auto shape = versionedObject->object;

		if (shape)
			DrawIndirectManager::UpdateInstanceBuffer(frameIndex, shape->GetBufferArrayIndex(), shape.use_count());
	}

	for (auto& [name, versionedObject] : shapes)
	{
		auto shape = versionedObject->object;

		bool versionChanged = false;

		if (shape && deviceAddressBuffers[frameIndex]->version != versionedObject->versions[frameIndex])
		{
			versionChanged = true;

			auto bufferHandler = static_cast<ShapeDeviceAddresses*>(deviceAddressBuffers[frameIndex]->buffer->GetHandler());
			bufferHandler[shape->GetBufferArrayIndex()] = ShapeDeviceAddresses{
				.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress(),
				.indexBufferAddress = shape->GetIndexBuffer()->GetAddress()
			};

			std::cout << std::format("Shape {} updated in frame {} with version {}", name, frameIndex, versionedObject->versions[frameIndex]) << std::endl;
		}

		//Todo: Handle model LOD???
		if (shape && indirectCommandBuffers[frameIndex]->version != versionedObject->versions[frameIndex])
		{
			versionChanged = true;

			auto bufferHandler = static_cast<VkDrawIndirectCommand*>(indirectCommandBuffers[frameIndex]->buffer->GetHandler());
			bufferHandler[shape->GetBufferArrayIndex()] = VkDrawIndirectCommand{
				.vertexCount = shape->GetIndexCount(),
				.instanceCount = 0,
				.firstVertex = 0,
				.firstInstance = 0,
			};

			std::cout << std::format("Shape drawIndirectCommand {} updated in frame {} with version {}", name, frameIndex, versionedObject->versions[frameIndex]) << std::endl;
		}

		//Important: The buffers will be resized and regenerated at the same time! So model->version can be used to handle both!!!
		if(versionChanged)
			versionedObject->versions[frameIndex] = indirectCommandBuffers[frameIndex]->version;
	}
}