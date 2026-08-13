// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <cstdint>

namespace Syn {
	constexpr uint32_t REGENERATE_BIT = 0;
	constexpr uint32_t UPDATE_BIT = 1;
	constexpr uint32_t CHANGED_BIT = 2;
	constexpr uint32_t INDEX_CHANGED_BIT = 3;
	constexpr uint32_t DIRTY_STATIC_BIT = 4;
	constexpr uint32_t FORCE_STATIC_GPU_UPLOAD = 5;
	constexpr uint32_t QUEUED_BIT = 6;
	constexpr uint32_t CUSTOM_CHANGED_BIT1 = 7;
	constexpr uint32_t CUSTOM_CHANGED_BIT2 = 8;
	constexpr uint32_t CUSTOM_CHANGED_BIT3 = 9;
}
