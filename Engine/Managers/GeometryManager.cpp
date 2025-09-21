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
	shapes["Sphere"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Quad"] = std::make_shared<Quad>();
	shapes["Quad"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Capsule"] = std::make_shared<Capsule>();
	shapes["Capsule"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cube"] = std::make_shared<Cube>();
	shapes["Cube"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cylinder"] = std::make_shared<Cylinder>();
	shapes["Cylinder"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Cone"] = std::make_shared<Cone>();
	shapes["Cone"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Pyramid"] = std::make_shared<Pyramid>();
	shapes["Pyramid"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["Torus"] = std::make_shared<Torus>();
	shapes["Torus"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["ProxySphere"] = std::make_shared<Sphere>(16);
	shapes["ProxySphere"]->SetBufferArrayIndex(GetAvailableIndex());

	shapes["ProxyCone"] = std::make_shared<Cone>(16);
	shapes["ProxyCone"]->SetBufferArrayIndex(GetAvailableIndex());

	for (auto& [name, shape] : shapes)
	{
		static_cast<ShapeDeviceAddresses*>(deviceAddresses->GetHandler())[shape->GetBufferArrayIndex()] = ShapeDeviceAddresses{
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
