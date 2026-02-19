#ifndef LITL_ENGINE_ECS_ENTITY_REGISTRY_H__
#define LITL_ENGINE_ECS_ENTITY_REGISTRY_H__

#include <cstdint>
#include <initializer_list>
#include <span>
#include <vector>

#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/entityRecord.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Internal static registry of all entities.
    /// 
    /// This class is NOT thread-safe and the implementation assumes it is being
    /// called from a single thread. Direct use of the registry is not recommended.
    /// 
    /// Instead it should be used indirectly through the ECS World or an ECS Command Buffer.
    /// </summary>
    class EntityRegistry
    {
    public:

        static Entity create() noexcept;
        static std::vector<Entity> createMany(uint32_t count) noexcept;

        static void destroy(Entity entity) noexcept;
        static void destroyMany(std::initializer_list<Entity> entities) noexcept;
        static void destroyMany(std::span<Entity const> entities) noexcept;

        static EntityRecord& getRecord(Entity entity) noexcept;
        static bool isAlive(Entity entity) noexcept;

        /// <summary>
        /// For internal or testing purposes only.
        /// </summary>
        static void clear() noexcept;

    protected:

    private:
    };
}

#endif