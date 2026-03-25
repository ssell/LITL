#ifndef LITL_ECS_SYSTEM_REGISTRY_H__
#define LITL_ECS_SYSTEM_REGISTRY_H__

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace LITL::ECS
{
    class SystemRegistry
    {
    public:

        template<ValidSystem S>
        static System* getSystem()
        {
            static System system;
            return &system;
        }

    protected:

    private:
    };
}

#endif