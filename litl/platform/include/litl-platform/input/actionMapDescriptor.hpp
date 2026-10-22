#ifndef LITL_PLATFORM_INPUT_ACTION_MAP_DESCRIPTOR_H__
#define LITL_PLATFORM_INPUT_ACTION_MAP_DESCRIPTOR_H__

#include <string>
#include <unordered_map>
#include <vector>

#include "litl-platform/input/inputSource.hpp"

namespace litl
{
    struct ActionMapDescriptor
    {
        /// <summary>
        /// The unique name of this map used both for display and to hash and serve as the key.
        /// </summary>
        std::string name;

        /// <summary>
        /// Should any actions not consumed by this map be propagated down the stack?
        /// </summary>
        bool propagateActions{ true };

        /// <summary>
        /// The cursor mode applied when this map is the active map at the top of the stack.
        /// </summary>
        CursorMode cursorMode{ CursorMode::Normal };

        /// <summary>
        /// A map of binding names to binding input sources.
        /// For example: "MoveForward" is bound to [Key::W, Key::Up].
        /// </summary>
        std::unordered_map<std::string, std::vector<InputSource>> bindings;

        [[nodiscard]] bool isValid() const noexcept
        {
            return !name.empty() && (cursorMode != CursorMode::Unknown);
        }
    };
}

#endif