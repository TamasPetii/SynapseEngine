#pragma once
#include "IProfiler.h"

namespace Syn {
    class SYN_API ICpuProfiler : public IProfiler {
    public:
        virtual void BeginFrame(uint32_t frameIndex) = 0;
        virtual uint32_t StartMeasurement(uint32_t frameIndex, const std::string& name) = 0;
        virtual void EndMeasurement(uint32_t frameIndex, uint32_t measurementIndex) = 0;
    };

    class SYN_API CpuProfileScope {
    public:
        CpuProfileScope(ICpuProfiler* profiler, uint32_t frameIndex, const std::string& name)
            : _profiler(profiler), _frameIndex(frameIndex) {
            if (_profiler) {
                _measurementIndex = _profiler->StartMeasurement(_frameIndex, name);
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