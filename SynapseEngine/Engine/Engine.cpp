#include "Engine.h"
#include "Logger/SynLog.h"
#include "Registry/Type/TypeInfo.h"

namespace Syn
{
	void Engine::Initialize()
	{
		Logger::Get().AddSink(std::make_shared<Syn::ConsoleSink>());
		Logger::Get().AddSink(std::make_shared<Syn::MemorySink>());
		Logger::Get().AddSink(std::make_shared<Syn::FileSink>());
	}
}