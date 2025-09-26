#pragma once
#include <array>
#include <memory>
#include "Engine/EngineApi.h"
#include "Engine/Config.h"


template<typename T>
struct VersionedObject
{
	VersionedObject(std::shared_ptr<T> object);

	std::shared_ptr<T> object = nullptr;
	std::array<uint32_t, GlobalConfig::FrameConfig::maxFramesInFlights> versions;
};

template<typename T>
VersionedObject<T>::VersionedObject(std::shared_ptr<T> object) :
	object(object) 
{
	for (uint32_t i = 0; i < versions.size(); i++)
		versions[i] = 0;
}