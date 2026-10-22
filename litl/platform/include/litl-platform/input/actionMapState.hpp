#ifndef LITL_PLATFORM_INPUT_ACTION_MAP_STATE_H__
#define LITL_PLATFORM_INPUT_ACTION_MAP_STATE_H__

namespace litl
{
    struct ActionMapState
    {
        bool live{ false };
        bool held{ false };
        bool pressed{ false };
        bool released{ false };
        bool triggered{ false };
        vec2 value{};
    };
}

#endif