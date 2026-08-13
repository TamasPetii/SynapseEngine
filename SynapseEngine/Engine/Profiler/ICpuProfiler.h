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
#include "IProfiler.h"

namespace Syn {
    class SYN_API ICpuProfiler : public IProfiler {
    public:
        virtual void BeginFrame(uint32_t frameIndex) = 0;
        virtual uint32_t StartMeasurement(uint32_t frameIndex, const std::string& groupName, const std::string& name) = 0;
        virtual void EndMeasurement(uint32_t frameIndex, uint32_t measurementIndex) = 0;
    };

    class SYN_API CpuProfileScope {
    public:
        CpuProfileScope(ICpuProfiler* profiler, uint32_t frameIndex, const std::string& groupName, const std::string& name)
            : _profiler(profiler), _frameIndex(frameIndex) {
            if (_profiler) {
                _measurementIndex = _profiler->StartMeasurement(_frameIndex, groupName, name);
            }
        }

        ~CpuProfileScope() {
            if (_profiler) {
                _profiler->EndMeasurement(_frameIndex, _measurementIndex);
            }
        }
    private:
        ICpuProfiler* _profiler;
        uint32_t _frameIndex;
        uint32_t _measurementIndex = 0;
    };

}