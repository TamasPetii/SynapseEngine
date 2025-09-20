#pragma once
#include "Engine/Renderable/Geometry/Geometries.h"
#include "BaseManagers/DeviceAddressedManager.h"

#include <memory>
#include <string>
#include <unordered_map>

struct ENGINE_API ShapeDeviceAddresses
{
	VkDeviceAddress vertexBufferAddress;
	VkDeviceAddress indexBufferAddress;
};

class ENGINE_API GeometryManager : public DeviceAddressedManager<ShapeDeviceAddresses>
{
public:
	GeometryManager();
	~GeometryManager();

	std::shared_ptr<Shape> GetShape(const std::string& name) const;
	const auto& GetShapes() { return shapes; }
private:
	void Initialize();
	void Cleanup();
private:
	std::unordered_map<std::string, std::shared_ptr<Shape>> shapes;
};

