#include "TagComponent.h"

namespace Syn
{
	TagComponent::TagComponent() :
		name("Entity"),
		tag("Untagged"),
		localEnabled(true),
		globalEnabled(true)
	{}

	TagComponent::TagComponent(const std::string& name, const std::string& tag) :
		name(name),
		tag(tag)
	{}
}