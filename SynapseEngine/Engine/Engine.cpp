#include "Engine.h"
#include "Logger/SynLog.h"
#include "ServiceLocator.h"
#include "Vk/Context.h"

namespace Syn
{
	Engine::Engine(const EngineInitParams& params)
	{
		Init(params);
	}

	Engine::~Engine()
	{
		Shutdown();
	}

	void Engine::Init(const EngineInitParams& params)
	{
		Logger::Get().AddSink(std::make_shared<Syn::ConsoleSink>());
		Logger::Get().AddSink(std::make_shared<Syn::MemorySink>());
		Logger::Get().AddSink(std::make_shared<Syn::FileSink>());

		Vk::ContextInitParams vkContextParams{
			.enableValidation = EnableValidation,
			.getSurfaceExtensionsCallback = params.getSurfaceExtensionsCallback,
			.createSurfaceCallback = params.createSurfaceCallback,
			.getWindowExtentCallback = [&]() -> VkExtent2D {
				auto [width, height] = params.getWindowExtentCallback();
				return VkExtent2D{ width, height };
			}
		};

		_vkContext = std::make_unique<Syn::Vk::Context>(vkContextParams);
		ServiceLocator::Provide(_vkContext.get());
	}

	void Engine::Shutdown() {
		ServiceLocator::Shutdown();
		_vkContext.reset();
	}
}