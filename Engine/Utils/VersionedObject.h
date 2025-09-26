#pragma once
#include <array>
#include <memory>
#include "Engine/EngineApi.h"
#include "Engine/Config.h"


template<typename T>
struct ENGINE_API VersionedObject
{
	VersionedObject(std::shared_ptr<T> object) : object(object) {}

	std::shared_ptr<T> object = nullptr;
	std::array<uint32_t, GlobalConfig::FrameConfig::maxFramesInFlights> versions;
};

