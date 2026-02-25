#ifndef LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__
#define LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__

#include <memory>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    class World;

    /// <summary>
    /// Raw systems are custom classes/structs defined by the user. Their only requirement is that they have a valid update method.
    /// A fully functioning system internally also needs things like archetype lists, etc.
    /// 
    /// This class wraps around the user-defined system execution kernel and provides the required internal functionality.
    /// </summary>
    class SystemWrapper
    {
    public:

        SystemWrapper(SystemTypeId systemTypeId);
        ~SystemWrapper();

        void run(World& world, float dt) const noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif