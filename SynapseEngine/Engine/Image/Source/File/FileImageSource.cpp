#include "FileImageSource.h"

namespace Syn
{
	std::optional<RawImage> FileImageSource::Produce()
	{
		if (!_loader) {
			return std::nullopt;
		}

		return _loader->LoadFile(_path);
	}
}