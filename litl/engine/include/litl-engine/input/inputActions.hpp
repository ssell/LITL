#ifndef LITL_ENGINE_INPUT_ACTIONS_H__
#define LITL_ENGINE_INPUT_ACTIONS_H__

#include <memory>

#include "litl-core/stringId.hpp"
#include "litl-engine/input/actionMap.hpp"

namespace litl
{
    class InputActions
    {
    public:

        InputActions();
        ~InputActions();

        InputActions(InputActions const&) = delete;
        InputActions operator=(InputActions const&) = delete;

        [[nodiscard]] ActionMap& getMap(StringId id) noexcept;
        bool pushMap(StringId id) noexcept;
        void popMap() noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif