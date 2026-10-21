#include "litl-engine/input/processedInputTracker.hpp"

namespace litl
{
    namespace
    {
        constexpr uint32_t OffsetKey = 0u;
        constexpr uint32_t OffsetMouseButton = static_cast<uint32_t>(Key::Count);
        constexpr uint32_t OffsetGamepadButton = OffsetMouseButton + static_cast<uint32_t>(MouseButton::Count);
        constexpr uint32_t OffsetGamepadAxis = OffsetGamepadButton + static_cast<uint32_t>(GamepadAxis::Count);

        [[nodiscard]] uint32_t getGeneralIndex(InputSource source) noexcept
        {
            switch (source.type)
            {
            case InputSourceType::Key:
                return (OffsetKey + source.code);

            case InputSourceType::MouseButton:
                return (OffsetMouseButton + source.code);

            case InputSourceType::GamepadButton:
                return (OffsetGamepadButton + source.code);

            case InputSourceType::GamepadAxis:
                return (OffsetGamepadAxis + source.code);

            default:
                return Constants::uint32_null_index;
            }
        }
    }
    void ProcessedInputTracker::reset() noexcept
    {
        m_processedGeneral.fill(false);
        m_processedMouseDelta = false;
        m_processedMouseScroll = false;
    }

    bool ProcessedInputTracker::isProcessed(InputSource source) const noexcept
    {
        switch (source.type)
        {
        case InputSourceType::Key:
        case InputSourceType::MouseButton:
        case InputSourceType::GamepadButton:
        case InputSourceType::GamepadAxis:
            {
                const auto index = getGeneralIndex(source);

                if (index != Constants::uint32_null_index)
                {
                    return m_processedGeneral[index];
                }
            }
            break;

        case InputSourceType::MouseDelta:
            return m_processedMouseDelta;

        case InputSourceType::MouseScroll:
            return m_processedMouseScroll;

        default:
            break;
        }

        // Return true so the action map doesn't try to process this unknown/unhandled type.
        return true;
    }

    void ProcessedInputTracker::markProcessed(InputSource source) noexcept
    {
        switch (source.type)
        {
        case InputSourceType::Key:
        case InputSourceType::MouseButton:
        case InputSourceType::GamepadButton:
        case InputSourceType::GamepadAxis:
        {
            const auto index = getGeneralIndex(source);

            if (index != Constants::uint32_null_index)
            {
                m_processedGeneral[index] = true;
            }
        }
        break;

        case InputSourceType::MouseDelta:
            m_processedMouseDelta = true;

        case InputSourceType::MouseScroll:
            m_processedMouseScroll = true;

        default:
            break;
        }
    }
}