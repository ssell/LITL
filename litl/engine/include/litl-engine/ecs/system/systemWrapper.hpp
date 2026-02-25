#ifndef LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__
#define LITL_ENGINE_ECS_SYSTEM_WRAPPER_H__

#include "litl-engine/ecs/constants.hpp"

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

        SystemWrapper(SystemTypeId type);
        SystemWrapper(SystemWrapper const&) = delete;
        SystemWrapper& operator=(SystemWrapper const&) = delete;

        SystemTypeId type() const noexcept;
        
        void run(World& world, float dt) const noexcept;

    protected:

    private:

        const SystemTypeId m_type;
    };
}

#endif