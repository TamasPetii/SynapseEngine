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
	shapes["Sphere"] = std::make_shared<Sphere>();
	shapes["Sphere"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Quad"] = std::make_shared<Quad>();
	shapes["Quad"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Capsule"] = std::make_shared<Capsule>();
	shapes["Capsule"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Cube"] = std::make_shared<Cube>();
	shapes["Cube"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Cylinder"] = std::make_shared<Cylinder>();
	shapes["Cylinder"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Cone"] = std::make_shared<Cone>();
	shapes["Cone"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Pyramid"] = std::make_shared<Pyramid>();
	shapes["Pyramid"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["Torus"] = std::make_shared<Torus>();
	shapes["Torus"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["ProxySphere"] = std::make_shared<Sphere>(16);
	shapes["ProxySphere"]->SetDescriptorArrayIndex(GetAvailableIndex());

	shapes["ProxyCone"] = std::make_shared<Cone>(16);
	shapes["ProxyCone"]->SetDescriptorArrayIndex(GetAvailableIndex());

	for (auto& [name, shape] : shapes)
	{
		static_cast<ShapeDeviceAddresses*>(deviceAddresses->GetHandler())[shape->GetDescriptorArrayIndex()] = ShapeDeviceAddresses{
			.vertexBufferAddress = shape->GetVertexBuffer()->GetAddress(),
			.indexBufferAddress = shape->GetIndexBuffer()->GetAddress()
		};
	}

}

void GeometryManager::Cleanup()
{
	shapes.clear();
}

std::shared_ptr<Shape> GeometryManager::GetShape(const std::string& name) const
{
	if (shapes.find(name) == shapes.end())
		return nullptr;

	return shapes.at(name);
}
