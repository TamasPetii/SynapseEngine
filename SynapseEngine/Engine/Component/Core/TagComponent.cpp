#include "TagComponent.h"

namespace Syn
{
	TagComponent::TagComponent() :
		name("Entity"),
		tag("Untagged"),
		enabled(true)
	{}

	TagComponent::TagComponent(const std::string& name, const std::string& tag) :
		name(name),
		tag(tag)
	{}
}