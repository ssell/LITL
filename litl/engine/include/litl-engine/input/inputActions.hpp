#ifndef LITL_ENGINE_INPUT_ACTIONS_H__
#define LITL_ENGINE_INPUT_ACTIONS_H__

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
    };
}

#endif