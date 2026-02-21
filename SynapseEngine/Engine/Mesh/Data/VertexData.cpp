#include "VertexData.h"

namespace Syn {
	void VertexPosition::SetMeshAndNodeIndex(uint16_t meshIndex, uint16_t nodeIndex)
	{
		packedIndex = (static_cast<uint32_t>(meshIndex) << 16) | static_cast<uint32_t>(nodeIndex);
	}
}