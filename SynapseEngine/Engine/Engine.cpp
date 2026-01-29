#include "Engine.h"
#include "Logger/SynLog.h"
#include "ServiceLocator.h"
#include "Vk/Context.h"
#include "Vk/Shader/Shader.h"
#include "Vk/Buffer/SynVkBuffer.h"

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

	void Engine::Update()
	{
	}

	void Engine::Render()
	{
		if (_isMinimized) 
			return;
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
			.getWindowExtentCallback = [=]() -> VkExtent2D {
				auto [width, height] = params.getWindowExtentCallback();
				return VkExtent2D{ width, height };
			}
		};

		_vkContext = std::make_unique<Syn::Vk::Context>(vkContextParams);
		ServiceLocator::ProvideVkContext(_vkContext.get());

		auto shader = std::make_shared<Vk::Shader>("../Engine/Shaders/Test.comp", VK_SHADER_STAGE_COMPUTE_BIT);
    }

	void Engine::Shutdown() {
		ServiceLocator::Shutdown();
		_vkContext.reset();
	}

	void Engine::WindowResizeEvent(uint32_t width, uint32_t height) {
		if (width == 0 || height == 0) {
			_isMinimized = true;
		}

		_isMinimized = false;
		_vkContext->GetSwapChain()->Recreate();
	}
}