#include "litl-engine/input/actionMap.hpp"

namespace litl
{
    struct ActionMap::Impl
    {

    };

    ActionMap::ActionMap()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    ActionMap::~ActionMap()
    {

    }

    bool ActionMap::bind(StringId name, InputSource source) noexcept
    {
        // ... todo ...

        return true;
    }
}