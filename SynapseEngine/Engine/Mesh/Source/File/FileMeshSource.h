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
#include "Engine/SynApi.h"
#include "../IMeshSource.h"
#include <optional>
#include <filesystem>

namespace Syn
{
	struct RawModel;
	class IMeshLoader;

	class SYN_API FileMeshSource : public IMeshSource
	{
	public:
		FileMeshSource(const std::filesystem::path& path, IMeshLoader* loader);
		virtual std::optional<RawModel> Produce() override;
	private:
		IMeshLoader* _loader;
		std::filesystem::path _path;
	};
}