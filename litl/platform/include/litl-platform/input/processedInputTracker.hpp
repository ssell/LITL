#ifndef LITL_PLATFORM_INPUT_PROCESSED_INPUT_TRACKER_H__
#define LITL_PLATFORM_INPUT_PROCESSED_INPUT_TRACKER_H__

#include <cstdint>
#include <array>

#include "litl-platform/input/inputSource.hpp"

namespace litl
{
    class ProcessedInputTracker
    {
    public:

        void reset() noexcept;
        [[nodiscard]] bool isProcessed(InputSource source) const noexcept;
        void markProcessed(InputSource source) noexcept;

    private:

        static constexpr uint32_t TotalInputSources = static_cast<uint32_t>(Key::Count) + static_cast<uint32_t>(MouseButton::Count) + static_cast<uint32_t>(GamepadButton::Count) + static_cast<uint32_t>(GamepadAxis::Count);
        
        std::array<bool, TotalInputSources> m_processedGeneral{};
        bool m_processedMouseDelta{ false };
        bool m_processedMouseScroll{ false };
    };
}

#endif