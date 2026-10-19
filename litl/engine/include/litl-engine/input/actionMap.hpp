#ifndef LITL_ENGINE_INPUT_ACTION_MAP_H__
#define LITL_ENGINE_INPUT_ACTION_MAP_H__

#include <cstdint>
#include <memory>

#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-engine/input/inputSource.hpp"

namespace litl
{
    enum class ActionType : uint8_t
    {
        Unknown = 0u,
        Button = 1u,
        Axis1D = 2u,
        Axis2D = 3u
    };

    struct ActionState
    {
        bool pressed{ false };
        bool released{ false };
        bool down{ false };
        vec2 value{};
    };

    class ActionMap
    {
    public:

        struct DefaultIds
        {
            static constexpr StringId UI = "UI"_sid;
            static constexpr StringId Gameplay = "Gameplay"_sid;
        };

        ActionMap();
        ~ActionMap();

        ActionMap(ActionMap const&) = delete;
        ActionMap& operator=(ActionMap const&) = delete;

        bool bind(StringId name, InputSource source) noexcept;
        void unbind(StringId name, InputSource source) noexcept;
        void unbindAll(StringId name) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif