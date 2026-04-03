#pragma once

namespace Syn {
    template<typename TState, typename TIntent>
    class IViewModel {
    public:
        virtual ~IViewModel() = default;
        virtual void SyncWithEngine() = 0;
        virtual const TState& GetState() const = 0;
        virtual void Dispatch(const TIntent& intent) = 0;
    };
}